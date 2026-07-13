#include "paris/editor.hpp"

#include "paris/callbacks.hpp"

#include <any>
#include <mutex>

namespace paris::editor {

namespace {
    std::mutex g_mtx;
    Backend    g_backend;

    template <typename Fn, typename... Args>
    auto call_or_default(Fn Backend::*mem, Args&&... args) {
        std::lock_guard lk(g_mtx);
        using R = decltype((g_backend.*mem)(std::forward<Args>(args)...));
        if (!(g_backend.*mem)) return R{};
        return (g_backend.*mem)(std::forward<Args>(args)...);
    }

    template <typename Fn, typename... Args>
    void call_void(Fn Backend::*mem, Args&&... args) {
        std::lock_guard lk(g_mtx);
        if ((g_backend.*mem)) (g_backend.*mem)(std::forward<Args>(args)...);
    }
}

void install(Backend b) {
    std::lock_guard lk(g_mtx);
    g_backend = std::move(b);
}

std::string get_active_file()         { return call_or_default(&Backend::get_active_file); }
std::string get_active_file_content() { return call_or_default(&Backend::get_active_file_content); }
std::string get_active_language()     { return call_or_default(&Backend::get_active_language); }
std::string get_root_path()           { return call_or_default(&Backend::get_root_path); }
std::string get_selection_text()      { return call_or_default(&Backend::get_selection_text); }

int get_cursor_line() { return call_or_default(&Backend::get_cursor_line); }
int get_cursor_col()  { return call_or_default(&Backend::get_cursor_col); }
int get_line_count()  { return call_or_default(&Backend::get_line_count); }
int get_tab_count()   { return call_or_default(&Backend::get_tab_count); }
int get_active_tab()  { return call_or_default(&Backend::get_active_tab); }

std::string              get_line_text(int line) { return call_or_default(&Backend::get_line_text, line); }
std::vector<std::string> get_open_files()        { return call_or_default(&Backend::get_open_files); }
std::string              get_tab_file(int index) { return call_or_default(&Backend::get_tab_file, index); }

void set_cursor_pos(int l, int c)                                    { call_void(&Backend::set_cursor_pos, l, c); }
void set_selection (int sl, int sc, int el, int ec)                  { call_void(&Backend::set_selection, sl, sc, el, ec); }
void insert_text       (const std::string& t)                        { call_void(&Backend::insert_text, t); }
void replace_selection (const std::string& t)                        { call_void(&Backend::replace_selection, t); }
void open_file         (const std::string& p)                        { call_void(&Backend::open_file, p); }
void save_active_file  ()                                            { call_void(&Backend::save_active_file); }
void goto_line         (int l)                                       { call_void(&Backend::goto_line, l); }
void show_notification (const std::string& m)                        { call_void(&Backend::show_notification, m); }
void set_status        (const std::string& m)                        { call_void(&Backend::set_status, m); }
void send_chat_message (const std::string& m)                        { call_void(&Backend::send_chat_message, m); }

// ---------- Event fan-out ----------

// Payload structs — scripts read them via callbacks::add_any.
struct FileEvent   { std::string path; };
struct BufferEvent { std::string path; int line; };

void on_file_opened(const std::string& p) {
    callbacks::fire_any("editor.file_opened", std::any{FileEvent{p}});
}
void on_file_saved(const std::string& p) {
    callbacks::fire_any("editor.file_saved", std::any{FileEvent{p}});
}
void on_buffer_changed(const std::string& p, int line) {
    callbacks::fire_any("editor.buffer_changed", std::any{BufferEvent{p, line}});
}
void on_tab_changed(const std::string& p) {
    callbacks::fire_any("editor.tab_changed", std::any{FileEvent{p}});
}

} // namespace paris::editor
