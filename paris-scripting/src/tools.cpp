#include "paris/tools.hpp"

#include "paris/callbacks.hpp"

#include <algorithm>
#include <mutex>
#include <unordered_map>

namespace paris::tools {

namespace {
    std::mutex g_mtx;
    std::unordered_map<std::string, Tool> g_tools;
}

void register_tool(const std::string& name, const std::string& desc, const std::string& params_json) {
    std::lock_guard lk(g_mtx);
    Tool t;
    t.name        = name;
    t.description = desc;
    t.params_json = params_json;
    t.section     = callbacks::detail::current_section();
    g_tools[name] = std::move(t);
}

void register_param(const std::string& tool, const std::string& name,
                    const std::string& type, const std::string& desc, bool required) {
    std::lock_guard lk(g_mtx);
    auto it = g_tools.find(tool);
    if (it == g_tools.end()) return;
    it->second.params.push_back({ name, type, desc, required });
}

void unregister(const std::string& name) {
    std::lock_guard lk(g_mtx);
    g_tools.erase(name);
}

std::vector<Tool> list() {
    std::lock_guard lk(g_mtx);
    std::vector<Tool> out;
    out.reserve(g_tools.size());
    for (auto& [_, t] : g_tools) out.push_back(t);
    return out;
}

void drop_section(const std::string& section) {
    std::lock_guard lk(g_mtx);
    for (auto it = g_tools.begin(); it != g_tools.end();) {
        if (it->second.section == section) it = g_tools.erase(it);
        else ++it;
    }
}

} // namespace paris::tools
