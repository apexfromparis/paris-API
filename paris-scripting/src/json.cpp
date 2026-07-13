#include "paris/json.hpp"

namespace paris::json {

std::optional<value> parse(const std::string& text) {
    try { return value::parse(text); } catch (...) { return std::nullopt; }
}

std::optional<value> decode(const std::string& text) { return parse(text); }

std::string stringify(const value& v, int indent) { return v.dump(indent); }
std::string encode   (const value& v, int indent) { return v.dump(indent); }

} // namespace paris::json
