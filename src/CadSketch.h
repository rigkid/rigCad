#pragma once

#include <string>

namespace rigkit {

class MEcs;
class rigManifold;
class rigPython;

namespace cad {

/**
 * @brief Install the `rigcad` Python module (box / difference / union / show).
 * @details Requires both embeds. `show` writes a triangle `CMesh` entity into `ecs`.
 */
bool installBindings(rigPython& py, rigManifold& man, MEcs* ecs, std::string* err = nullptr);

/**
 * @brief Eval sketch source with bindings installed.
 * @return false when embeds missing or Python failed.
 */
bool runSketch(rigPython& py, rigManifold& man, MEcs* ecs, const std::string& source,
			   std::string* err = nullptr);

inline constexpr const char* kDefaultSketchPath = "sketch.py";

inline constexpr const char* kStarterSketch =
	"# rigCad sketch — Manifold CSG via rigcad\n"
	"from rigcad import box, difference, show\n"
	"show(difference(box(40, 30, 20), box(20, 14, 12)))\n";

} // namespace cad
} // namespace rigkit
