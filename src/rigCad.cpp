#include "rigCad.h"

#include <spdlog/spdlog.h>
#include "CadSketch.h"
#include "CodeEditorWindow.h"
#include "MWindow.h"
#include "core/IMui.h"
#include "core/RigKitEngine.h"
#include "core/pack/MPack.h"
#include "core/pack/PackRegistry.h"
#include "rigManifold.h"
#include "rigPython.h"

namespace rigkit {

rigCad::rigCad() : IPack("rigCad") {
	

}

bool rigCad::init() {
	spdlog::info("[rigCad] init");
	return true;
}

void rigCad::setup() {
	auto* engine = getEngine();
	if (!engine) {
		return;
	}

	auto* packs = engine->getPackManager();
	std::shared_ptr<rigPython> py;
	std::shared_ptr<rigManifold> man;
	if (packs) {
		py = std::dynamic_pointer_cast<rigPython>(packs->getPack("rigPython"));
		man = std::dynamic_pointer_cast<rigManifold>(packs->getPack("rigManifold"));
	}
	m_python = py;
	m_manifold = man;

	auto* ui = engine->getUiManager();
	if (ui && ui->getWindowManager()) {
		if (auto ed = ui->getWindowManager()->getWindow<CodeEditorWindow>("Code Editor")) {
			ed->setText(cad::kStarterSketch, TextEditor::LanguageDefinitionId::Python);
		}
	}

	m_ready = py && py->available() && man && man->available();
	if (!m_ready) {
		m_lastError = "waiting for rigPython + rigManifold embeds";
		spdlog::info("[rigCad] setup — ready=false ({})", m_lastError);
		return;
	}
	m_lastError.clear();
	spdlog::info("[rigCad] setup — ready=true");

	std::string err;
	if (!cad::installBindings(*py, *man, engine->getECSManager(), &err)) {
		m_ready = false;
		m_lastError = err;
		spdlog::warn("[rigCad] bindings failed: {}", err);
		return;
	}

	if (!runSketch(cad::kStarterSketch)) {
		spdlog::warn("[rigCad] starter sketch failed: {}", m_lastError);
	}
}

bool rigCad::runSketch(const std::string& source) {
	auto py = std::dynamic_pointer_cast<rigPython>(m_python.lock());
	auto man = std::dynamic_pointer_cast<rigManifold>(m_manifold.lock());
	auto* engine = getEngine();
	if (!py || !man || !engine) {
		m_lastError = "packs not available";
		return false;
	}
	std::string err;
	if (!cad::runSketch(*py, *man, engine->getECSManager(), source, &err)) {
		m_lastError = err;
		return false;
	}
	m_lastError.clear();
	return true;
}

} // namespace rigkit

namespace {
struct cadRegistrar {
	cadRegistrar() {
		rigkit::PackRegistry::instance().addFactory("rigCad", []() {
			return std::shared_ptr<rigkit::IPack>(std::make_shared<rigkit::rigCad>());
		});
	}
};
static cadRegistrar cad_auto_reg;
} // namespace
