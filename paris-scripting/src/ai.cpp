#include "paris/ai.hpp"

#include "paris/callbacks.hpp"

#include <algorithm>
#include <mutex>

namespace paris::ai {

namespace {
    template <typename Fn>
    struct Entry { std::string section; Fn fn; };

    std::mutex g_mtx;
    std::vector<Entry<before_send_fn_t>>    g_before_send;
    std::vector<Entry<after_response_fn_t>> g_after_response;
    std::vector<Entry<tool_call_fn_t>>      g_tool_call;
    std::vector<Entry<after_tool_fn_t>>     g_after_tool;
    std::vector<Entry<system_inject_fn_t>>  g_system_inject;

    std::string           g_model = "";
    std::vector<Message>  g_history;

    template <typename Vec, typename Fn>
    void add_to(Vec& v, Fn fn) {
        if (!fn) return;
        std::lock_guard lk(g_mtx);
        v.push_back({ callbacks::detail::current_section(), std::move(fn) });
    }
    template <typename Vec>
    void drop_from(Vec& v, const std::string& section) {
        v.erase(std::remove_if(v.begin(), v.end(),
            [&](const auto& e){ return e.section == section; }), v.end());
    }
}

void on_before_send   (before_send_fn_t fn)   { add_to(g_before_send,    std::move(fn)); }
void on_after_response(after_response_fn_t fn){ add_to(g_after_response, std::move(fn)); }
void on_tool_call     (tool_call_fn_t fn)     { add_to(g_tool_call,      std::move(fn)); }
void on_after_tool    (after_tool_fn_t fn)    { add_to(g_after_tool,     std::move(fn)); }
void on_system_inject (system_inject_fn_t fn) { add_to(g_system_inject,  std::move(fn)); }

bool fire_before_send(std::string& prompt, std::string& system_prompt) {
    std::vector<before_send_fn_t> fns;
    { std::lock_guard lk(g_mtx); for (auto& e : g_before_send) fns.push_back(e.fn); }
    for (auto& fn : fns) if (!fn(prompt, system_prompt)) return false;
    return true;
}

void fire_after_response(std::string& response) {
    std::vector<after_response_fn_t> fns;
    { std::lock_guard lk(g_mtx); for (auto& e : g_after_response) fns.push_back(e.fn); }
    for (auto& fn : fns) fn(response);
}

std::string fire_tool_call(const std::string& name, const std::string& args_json) {
    std::vector<tool_call_fn_t> fns;
    { std::lock_guard lk(g_mtx); for (auto& e : g_tool_call) fns.push_back(e.fn); }
    for (auto& fn : fns) {
        auto r = fn(name, args_json);
        if (!r.empty()) return r;
    }
    return {};
}

void fire_after_tool(const std::string& name, const std::string& args_json,
                     const std::string& result) {
    std::vector<after_tool_fn_t> fns;
    { std::lock_guard lk(g_mtx); for (auto& e : g_after_tool) fns.push_back(e.fn); }
    for (auto& fn : fns) fn(name, args_json, result);
}

std::string fire_system_inject() {
    std::vector<system_inject_fn_t> fns;
    { std::lock_guard lk(g_mtx); for (auto& e : g_system_inject) fns.push_back(e.fn); }
    std::string out;
    for (auto& fn : fns) {
        auto s = fn();
        if (!s.empty()) { if (!out.empty()) out += "\n"; out += s; }
    }
    return out;
}

std::string get_active_model()                          { std::lock_guard lk(g_mtx); return g_model; }
void        set_active_model(const std::string& m)      { std::lock_guard lk(g_mtx); g_model = m; }
int         get_chat_message_count()                    { std::lock_guard lk(g_mtx); return int(g_history.size()); }
Message     get_chat_message(int i) {
    std::lock_guard lk(g_mtx);
    if (i < 0 || i >= int(g_history.size())) return {};
    return g_history[i];
}
void set_chat_history(std::vector<Message> h) {
    std::lock_guard lk(g_mtx); g_history = std::move(h);
}

void drop_section(const std::string& section) {
    std::lock_guard lk(g_mtx);
    drop_from(g_before_send,    section);
    drop_from(g_after_response, section);
    drop_from(g_tool_call,      section);
    drop_from(g_after_tool,     section);
    drop_from(g_system_inject,  section);
}

} // namespace paris::ai
