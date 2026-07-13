#include "paris/fs.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

#ifdef _WIN32
    #include <shlobj.h>
#endif

namespace paris::fs {

namespace {
    std::mutex        g_mtx;
    std::string       g_root;

    std::string default_root() {
#ifdef _WIN32
        wchar_t* out = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &out))) {
            std::wstring w(out);
            CoTaskMemFree(out);
            std::string s(w.begin(), w.end());
            return s + "\\My Games\\paris";
        }
#endif
        return "./paris_scripts_data";
    }

    // Reject absolute paths and any `..` component. Return the canonical relative
    // string on success, empty on failure.
    std::string sanitize(const std::string& rel) {
        if (rel.empty()) return {};
        std::filesystem::path p(rel);
        if (p.is_absolute()) return {};
        std::string clean;
        for (auto& part : p) {
            auto s = part.string();
            if (s == ".." || s == "" ) return {};
            if (!clean.empty()) clean += '/';
            clean += s;
        }
        return clean;
    }

    std::filesystem::path abs_of(const std::string& rel) {
        std::lock_guard lk(g_mtx);
        auto root = g_root.empty() ? default_root() : g_root;
        auto safe = sanitize(rel);
        if (safe.empty()) return {};
        std::filesystem::create_directories(root);
        return std::filesystem::path(root) / safe;
    }
}

void        set_root(const std::string& r) { std::lock_guard lk(g_mtx); g_root = r; }
std::string get_root() {
    std::lock_guard lk(g_mtx);
    return g_root.empty() ? default_root() : g_root;
}

bool create_directory(const std::string& rel) {
    auto p = abs_of(rel); if (p.empty()) return false;
    std::error_code ec;
    std::filesystem::create_directories(p, ec);
    return !ec;
}

bool create_file(const std::string& rel, const std::string& data) {
    auto p = abs_of(rel); if (p.empty()) return false;
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(data.data(), data.size());
    return f.good();
}

bool does_file_exist(const std::string& rel) {
    auto p = abs_of(rel); if (p.empty()) return false;
    std::error_code ec;
    return std::filesystem::exists(p, ec) && !ec;
}

ReadResult read_file(const std::string& rel) {
    ReadResult r;
    auto p = abs_of(rel); if (p.empty()) return r;
    std::ifstream f(p, std::ios::binary);
    if (!f) return r;
    std::ostringstream ss; ss << f.rdbuf();
    r.data = ss.str(); r.ok = true;
    return r;
}

bool delete_file(const std::string& rel) {
    auto p = abs_of(rel); if (p.empty()) return false;
    std::error_code ec;
    return std::filesystem::remove(p, ec) && !ec;
}

bool delete_directory(const std::string& rel) {
    auto p = abs_of(rel); if (p.empty()) return false;
    std::error_code ec;
    return std::filesystem::remove_all(p, ec) > 0 && !ec;
}

std::vector<QueryEntry> query_directory(const std::string& rel, bool inc_dirs,
                                        bool inc_files, const std::vector<std::string>& exts) {
    std::vector<QueryEntry> out;
    auto p = abs_of(rel); if (p.empty()) return out;
    std::error_code ec;
    if (!std::filesystem::exists(p, ec) || ec) return out;
    for (auto& it : std::filesystem::directory_iterator(p, ec)) {
        QueryEntry e;
        e.name         = it.path().filename().string();
        e.is_directory = it.is_directory(ec);
        e.size         = e.is_directory ? 0 : uint64_t(it.file_size(ec));
        if (e.is_directory && !inc_dirs)  continue;
        if (!e.is_directory && !inc_files) continue;
        if (!e.is_directory && !exts.empty()) {
            auto ext = it.path().extension().string();
            if (std::find(exts.begin(), exts.end(), ext) == exts.end()) continue;
        }
        out.push_back(std::move(e));
    }
    return out;
}

} // namespace paris::fs
