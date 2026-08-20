#include <memory>
#include "app.h"
#include "core/RigKitEngine.h"

int main(int argc, char* argv[]) {
	auto app = std::make_unique<CadApp>();
	auto* raw = app.get();
	rigkit::RigKitEngine engine(std::move(app), {}, argc, argv);
	engine.run();
	return raw->smokeFailed() ? 1 : 0;
}
