#pragma once
#include "core/U_core.h"
#include "core/util/CommandLineArgs.h"

class CadApp : public rigkit::IApp {
  public:
	CadApp() {
		window().width = 1100;
		window().height = 720;
		window().title = "rigCad — cad";
	}
	void parseCommandLineArgs(const rigkit::CommandLineArgs& args) override;
	void setup() override;
	void update(float) override {}
	void draw() override {}
	bool smokeFailed() const { return m_smoke && !m_smokeOk; }

  private:
	bool m_smoke = false;
	bool m_smokeOk = false;
};
