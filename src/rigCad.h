#pragma once

#include <memory>
#include <string>

#include "core/pack/IPack.h"

namespace rigkit {

/**
 * @brief Live-scripted 3D CAD orchestration.
 * @details Wires **rigCodeEditor** + **rigPython** + **rigManifold**. When both
 * embeds report `available()`, `ready()` is true and sketches can `show()` solids.
 */
class rigCad : public IPack {
  public:
	rigCad();
	bool init() override;
	void setup() override;

	/** @brief True when Python + Manifold embeds report available(). */
	bool ready() const { return m_ready; }

	const std::string& lastError() const { return m_lastError; }

	/** @brief Run `source` through the `rigcad` sketch bindings. */
	bool runSketch(const std::string& source);

  private:
	bool m_ready = false;
	std::string m_lastError;
	std::weak_ptr<IPack> m_python;
	std::weak_ptr<IPack> m_manifold;
};

} // namespace rigkit
