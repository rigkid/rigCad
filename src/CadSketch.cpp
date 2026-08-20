#include "CadSketch.h"

#include <new>
#include <spdlog/spdlog.h>

#include "ManifoldMesh.h"
#include "ecs/MEcs.h"
#include "rig/create.h"
#include "rigManifold.h"
#include "rigPython.h"

#if defined(RIGPYTHON_HAS_EMBED) && defined(RIGMANIFOLD_HAS_KERNEL)
#include <Python.h>
#include <manifold/manifold.h>
#endif

namespace rigkit {
namespace cad {
namespace {

#if defined(RIGPYTHON_HAS_EMBED) && defined(RIGMANIFOLD_HAS_KERNEL)

struct CadRuntime {
	MEcs* ecs = nullptr;
};

CadRuntime* g_rt = nullptr;

::manifold::Manifold* capsuleSolid(PyObject* obj) {
	return static_cast<::manifold::Manifold*>(PyCapsule_GetPointer(obj, "rigcad.Manifold"));
}

void freeSolidCapsule(PyObject* cap) {
	auto* p = static_cast<::manifold::Manifold*>(PyCapsule_GetPointer(cap, "rigcad.Manifold"));
	delete p;
}

PyObject* makeSolidCapsule(::manifold::Manifold solid) {
	auto* heap = new (std::nothrow)::manifold::Manifold(std::move(solid));
	if (!heap) {
		PyErr_NoMemory();
		return nullptr;
	}
	return PyCapsule_New(heap, "rigcad.Manifold", freeSolidCapsule);
}

PyObject* py_box(PyObject*, PyObject* args) {
	double sx = 1, sy = 1, sz = 1;
	int center = 1;
	if (!PyArg_ParseTuple(args, "ddd|p", &sx, &sy, &sz, &center)) {
		return nullptr;
	}
	return makeSolidCapsule(manifold::cube(sx, sy, sz, center != 0));
}

PyObject* py_difference(PyObject*, PyObject* args) {
	PyObject* aObj = nullptr;
	PyObject* bObj = nullptr;
	if (!PyArg_ParseTuple(args, "OO", &aObj, &bObj)) {
		return nullptr;
	}
	auto* a = capsuleSolid(aObj);
	auto* b = capsuleSolid(bObj);
	if (!a || !b) {
		return nullptr;
	}
	return makeSolidCapsule(manifold::difference(*a, *b));
}

PyObject* py_union(PyObject*, PyObject* args) {
	PyObject* aObj = nullptr;
	PyObject* bObj = nullptr;
	if (!PyArg_ParseTuple(args, "OO", &aObj, &bObj)) {
		return nullptr;
	}
	auto* a = capsuleSolid(aObj);
	auto* b = capsuleSolid(bObj);
	if (!a || !b) {
		return nullptr;
	}
	return makeSolidCapsule(manifold::unite(*a, *b));
}

PyObject* py_show(PyObject*, PyObject* args) {
	PyObject* obj = nullptr;
	if (!PyArg_ParseTuple(args, "O", &obj)) {
		return nullptr;
	}
	auto* solid = capsuleSolid(obj);
	if (!solid) {
		return nullptr;
	}
	if (!g_rt || !g_rt->ecs) {
		PyErr_SetString(PyExc_RuntimeError, "rigcad show: no ECS");
		return nullptr;
	}
	ecs::CMesh mesh;
	std::string err;
	if (!manifold::toCMesh(*solid, mesh, &err)) {
		PyErr_SetString(PyExc_RuntimeError, err.empty() ? "toCMesh failed" : err.c_str());
		return nullptr;
	}
	rig::makeMesh(*g_rt->ecs, std::move(mesh.positions), std::move(mesh.indices), mesh.mode,
				  rig::fill(0.85f, 0.55f, 0.25f), "cad-show");
	Py_RETURN_NONE;
}

PyMethodDef kMethods[] = {
	{"box", py_box, METH_VARARGS, "box(sx, sy, sz, center=True) -> solid"},
	{"difference", py_difference, METH_VARARGS, "difference(a, b) -> solid"},
	{"union", py_union, METH_VARARGS, "union(a, b) -> solid"},
	{"show", py_show, METH_VARARGS, "show(solid) — spawn CMesh entity"},
	{nullptr, nullptr, 0, nullptr},
};

PyModuleDef kModule = {
	PyModuleDef_HEAD_INIT, "rigcad", "RigKit CAD sketch helpers", -1, kMethods,
	nullptr,	 nullptr,			nullptr,					 nullptr,
};

PyObject* createRigcadModule() {
	return PyModule_Create(&kModule);
}

bool injectModule() {
	if (!Py_IsInitialized()) {
		return false;
	}
	PyObject* name = PyUnicode_FromString("rigcad");
	PyObject* existing = name ? PyImport_GetModule(name) : nullptr;
	Py_XDECREF(name);
	if (existing) {
		Py_DECREF(existing);
		return true;
	}
	PyObject* mod = createRigcadModule();
	if (!mod) {
		return false;
	}
	PyObject* sysModules = PyImport_GetModuleDict();
	if (!sysModules || PyDict_SetItemString(sysModules, "rigcad", mod) != 0) {
		Py_DECREF(mod);
		return false;
	}
	Py_DECREF(mod);
	return true;
}

#endif

} // namespace

bool installBindings(rigPython& py, rigManifold& man, MEcs* ecs, std::string* err) {
#if !(defined(RIGPYTHON_HAS_EMBED) && defined(RIGMANIFOLD_HAS_KERNEL))
	(void)py;
	(void)man;
	(void)ecs;
	if (err) {
		*err = "rigCad bindings need CPython + Manifold embeds";
	}
	return false;
#else
	if (!py.available() || !man.available()) {
		if (err) {
			*err = "Python or Manifold not available";
		}
		return false;
	}
	// Touch interpreter (may already be running from rigPython::init).
	if (!py.eval("pass")) {
		if (err) {
			*err = py.lastError();
		}
		return false;
	}
	static CadRuntime rt;
	rt.ecs = ecs;
	g_rt = &rt;
	if (!injectModule()) {
		if (err) {
			*err = "failed to inject rigcad module";
		}
		return false;
	}
	if (!py.eval("import rigcad")) {
		if (err) {
			*err = py.lastError();
		}
		return false;
	}
	(void)man;
	return true;
#endif
}

bool runSketch(rigPython& py, rigManifold& man, MEcs* ecs, const std::string& source,
			   std::string* err) {
	if (!installBindings(py, man, ecs, err)) {
		return false;
	}
	if (!py.eval(source)) {
		if (err) {
			*err = py.lastError();
		}
		return false;
	}
	return true;
}

} // namespace cad
} // namespace rigkit
