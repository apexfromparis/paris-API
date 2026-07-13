#include "paris/callbacks.hpp"

#include <algorithm>
#include <mutex>
#include <unordered_map>

namespace paris::callbacks {

namespace {
    struct Entry     { std::string section; fn_t     fn; };
    struct EntryAny  { std::string section; fn_any_t fn; };

    std::mutex g_mtx;
    std::unordered_map<std::string, std::vector<Entry>>    g_plain; // event → callbacks
    std::unordered_map<std::string, std::vector<EntryAny>> g_typed;

    thread_local std::vector<std::string> tl_section_stack;
}

namespace detail {
    void        push_section(std::string name) { tl_section_stack.push_back(std::move(name)); }
    void        pop_section()                  { if (!tl_section_stack.empty()) tl_section_stack.pop_back(); }
    std::string current_section()              { return tl_section_stack.empty() ? std::string{} : tl_section_stack.back(); }
}

void add(std::string_view event, fn_t fn) {
    if (!fn) return;
    std::lock_guard lk(g_mtx);
    g_plain[std::string(event)].push_back({ detail::current_section(), std::move(fn) });
}

void add_any(std::string_view event, fn_any_t fn) {
    if (!fn) return;
    std::lock_guard lk(g_mtx);
    g_typed[std::string(event)].push_back({ detail::current_section(), std::move(fn) });
}

void fire(std::string_view event) {
    std::vector<fn_t> to_run;
    {
        std::lock_guard lk(g_mtx);
        auto it = g_plain.find(std::string(event));
        if (it == g_plain.end()) return;
        to_run.reserve(it->second.size());
        for (auto& e : it->second) to_run.push_back(e.fn);
    }
    for (auto& fn : to_run) fn();
}

void fire_any(std::string_view event, const std::any& payload) {
    std::vector<fn_any_t> to_run;
    {
        std::lock_guard lk(g_mtx);
        auto it = g_typed.find(std::string(event));
        if (it == g_typed.end()) return;
        to_run.reserve(it->second.size());
        for (auto& e : it->second) to_run.push_back(e.fn);
    }
    for (auto& fn : to_run) fn(payload);
}

void drop_section(const std::string& section) {
    std::lock_guard lk(g_mtx);
    auto strip = [&](auto& map) {
        for (auto& [_, v] : map) {
            v.erase(std::remove_if(v.begin(), v.end(),
                [&](const auto& e){ return e.section == section; }), v.end());
        }
    };
    strip(g_plain);
    strip(g_typed);
}

} // namespace paris::callbacks
