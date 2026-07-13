#include "paris/metadata.hpp"

#include <algorithm>
#include <fstream>
#include <string>

namespace paris {

namespace {
    std::string trim(std::string s) {
        auto not_ws = [](char c){ return c != ' ' && c != '\t' && c != '\r'; };
        auto lo = std::find_if(s.begin(),  s.end(),  not_ws);
        auto hi = std::find_if(s.rbegin(), s.rend(), not_ws).base();
        return (lo < hi) ? std::string(lo, hi) : std::string{};
    }

    // Strips a leading comment marker if present, otherwise returns "" — that
    // means the line isn't a comment and should end the scan.
    std::string strip_comment(const std::string& raw) {
        auto s = raw;
        // Skip leading whitespace.
        size_t i = 0;
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
        if (i + 1 >= s.size()) return {};
        if (s.compare(i, 2, "--") == 0) return trim(s.substr(i + 2));
        if (s.compare(i, 2, "//") == 0) return trim(s.substr(i + 2));
        return {};
    }
}

ScriptMetadata parse_script_metadata(const std::filesystem::path& file) {
    ScriptMetadata md;
    std::ifstream f(file);
    if (!f) return md;

    std::string line;
    int scanned = 0;
    while (scanned < 30 && std::getline(f, line)) {
        ++scanned;
        auto stripped = strip_comment(line);
        // Empty comment or non-comment line stops the scan — metadata is
        // expected to live in the file header block.
        if (stripped.empty()) {
            // Empty comment is OK, non-comment stops us.
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            if (strip_comment(line).empty() &&
                line.find("--") == std::string::npos &&
                line.find("//") == std::string::npos) break;
            continue;
        }
        if (stripped[0] != '@') continue;

        auto space = stripped.find(' ');
        if (space == std::string::npos) continue;
        auto key = stripped.substr(1, space - 1);
        auto val = trim(stripped.substr(space + 1));

        if      (key == "name")        md.name        = val;
        else if (key == "author")      md.author      = val;
        else if (key == "version")     md.version     = val;
        else if (key == "description") md.description = val;
    }
    return md;
}

} // namespace paris
