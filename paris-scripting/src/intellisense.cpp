#include "paris/intellisense.hpp"

#include "paris/callbacks.hpp"

#include <algorithm>
#include <mutex>

namespace paris::intellisense {

namespace {
    struct CompEntry  { std::string section; completion_fn_t fn; };
    struct HoverEntry { std::string section; hover_fn_t      fn; };

    std::mutex g_mtx;
    std::vector<CompEntry>  g_completion;
    std::vector<HoverEntry> g_hover;
}

void register_completion(completion_fn_t fn) {
    if (!fn) return;
    std::lock_guard lk(g_mtx);
    g_completion.push_back({ callbacks::detail::current_section(), std::move(fn) });
}

void register_hover(hover_fn_t fn) {
    if (!fn) return;
    std::lock_guard lk(g_mtx);
    g_hover.push_back({ callbacks::detail::current_section(), std::move(fn) });
}

std::vector<Completion> collect_completions(const std::string& file,
                                            const std::string& line, int col) {
    std::vector<completion_fn_t> fns;
    {
        std::lock_guard lk(g_mtx);
        for (auto& e : g_completion) fns.push_back(e.fn);
    }
    std::vector<Completion> out;
    for (auto& fn : fns) {
        auto batch = fn(file, line, col);
        out.insert(out.end(), batch.begin(), batch.end());
    }
    return out;
}

std::string collect_hover(const std::string& file, const std::string& word, int line) {
    std::vector<hover_fn_t> fns;
    {
        std::lock_guard lk(g_mtx);
        for (auto& e : g_hover) fns.push_back(e.fn);
    }
    // First non-empty wins — matches how LSPs usually merge hover data.
    for (auto& fn : fns) {
        auto s = fn(file, word, line);
        if (!s.empty()) return s;
    }
    return {};
}

void drop_section(const std::string& section) {
    std::lock_guard lk(g_mtx);
    g_completion.erase(std::remove_if(g_completion.begin(), g_completion.end(),
        [&](const CompEntry& e){ return e.section == section; }), g_completion.end());
    g_hover.erase(std::remove_if(g_hover.begin(), g_hover.end(),
        [&](const HoverEntry& e){ return e.section == section; }), g_hover.end());
}

} // namespace paris::intellisense
