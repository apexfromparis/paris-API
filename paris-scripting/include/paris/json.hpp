#pragma once

#include <optional>
#include <string>

// Thin re-export of nlohmann::json plus the two functions perception.cx
// exposes to scripts. Both encode/parse pairs are aliases so a script written
// against either API name works.

#include <nlohmann/json.hpp>

namespace paris::json {

using value = nlohmann::json;

std::optional<value>       parse    (const std::string& text);
std::optional<value>       decode   (const std::string& text);
std::string                stringify(const value& v, int indent = -1);
std::string                encode   (const value& v, int indent = -1);

} // namespace paris::json
