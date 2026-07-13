#pragma once

#include <string>

namespace paris::autosave {

// Serialise the entire UI tree via `ui::construct_config` and write it to
// `relative_path` under the fs sandbox. Returns false if the write failed.
bool save(const std::string& relative_path = "configs/autosave.cfg");

// Read `relative_path` under the fs sandbox and apply the blob to the UI
// tree via `ui::apply_config`. Returns false when the file is missing.
bool load(const std::string& relative_path = "configs/autosave.cfg");

// Wire up the two calls to the engine's tick / shutdown lifecycle: `load` on
// the first tick after a set of scripts has finished loading, `save` on
// shutdown. Call once after loading the initial script directory.
void install_hooks(const std::string& relative_path = "configs/autosave.cfg");

} // namespace paris::autosave
