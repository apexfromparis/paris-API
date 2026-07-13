#pragma once

#include <filesystem>
#include <string>

namespace paris {

// Header-comment metadata for a script. All fields optional. Parse with
// `parse_script_metadata` — cheap, single-pass over the first N lines.
struct ScriptMetadata {
    std::string name;
    std::string author;
    std::string version;
    std::string description;
};

// Reads the first ~30 lines of a .lua or .as file and picks out lines that
// look like `-- @field value` or `// @field value`. Unknown fields are
// ignored. Returns an empty ScriptMetadata if the file can't be read.
ScriptMetadata parse_script_metadata(const std::filesystem::path& file);

} // namespace paris
