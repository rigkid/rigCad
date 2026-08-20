#include "app.h"

#include "rendering/U_gladGlfw.h"
#include <spdlog/spdlog.h>
#include "CLight.h"
#include "CTransform.h"
#include "core/RigKitEngine.h"
#include "core/pack/MPack.h"
#include "packs/rigCad/src/rigCad.h"
#include "packs/rigCodeEditor/src/rigCodeEditor.h"
#include "packs/rigComponent/src/rig.h"
#include "packs/rigComponent/src/rigComponent.h"
#include "packs/rigImGui/src/rigImGui.h"
#include "packs/rigManifold/src/rigManifold.h"
#include "packs/rigPython/src/rigPython.h"
#include "packs/rigRender3D/src/rigRender3D.h"
#include "packs/rigSystems/src/rigSystems.h"

void CadApp::parseCommandLineArgs(const rigkit::CommandLineArgs& args) {
	IApp::parseCommandLineArgs(args);
	if (args.hasFlag("smoke")) {
		m_smoke = true;
	}
}

void CadApp::setup() {
	spdlog::info("cad — Python + Manifold sketch orchestration");
	m_engine->setClearColor(0.10f, 0.11f, 0.14f, 1.0f);

	auto* packs = m_engine->getPackManager();
	if (!packs) {
		return;
	}
	packs->registerPack(std::make_shared<rigkit::rigComponent>());
	packs->registerPack(std::make_shared<rigkit::rigSystems>());
	packs->registerPack(std::make_shared<rigkit::rigRender3D>());
	packs->registerPack(std::make_shared<rigkit::rigImGui>());
	packs->registerPack(std::make_shared<rigkit::rigCodeEditor>());
	packs->registerPack(std::make_shared<rigkit::rigPython>());
	packs->registerPack(std::make_shared<rigkit::rigManifold>());
	auto cad = std::make_shared<rigkit::rigCad>();
	packs->registerPack(cad);
	packs->initAll();
	packs->setupAll();

	auto* ecs = m_engine->getECSManager();
	if (ecs) {
		auto cam = rig::makeCamera(*ecs, {0.0f, 25.0f, 70.0f}, true, "cad-camera");
		rig::lookAt(ecs->getComponent<rigkit::ecs::CTransform>(cam), {0.0f, 25.0f, 70.0f},
					{0.0f, 0.0f, 0.0f});
		auto light =
			rig::makeLight(*ecs, {40.0f, 60.0f, 30.0f}, rigkit::ecs::CLight::Type::Point, "cad-light");
		auto& ld = ecs->getComponent<rigkit::ecs::CLight>(light);
		ld.intensity = 1.3f;
		ld.ambient = 0.3f;
	}

	m_smokeOk = cad->ready();
	spdlog::info("cad — ready={} ({})", cad->ready(), cad->lastError());
	if (!m_smokeOk) {
		spdlog::error("cad — not ready");
	}

	if (m_smoke) {
		if (auto* win = m_engine->getWindow()) {
			glfwSetWindowShouldClose(win, GLFW_TRUE);
		}
	}
}
