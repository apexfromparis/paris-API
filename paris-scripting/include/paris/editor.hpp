#pragma once

#include <functional>
#include <string>
#include <vector>

namespace paris::editor {

// ---------- Query (script → host) ----------

// Returns "" when the query cannot be satisfied (no active tab, no backend
// installed, etc.). Scripts should treat empty strings as absence.

std::string get_active_file();
std::string get_active_file_content();
std::string get_active_language();
std::string get_root_path();
std::string get_selection_text();

int  get_cursor_line();
int  get_cursor_col();
int  get_line_count();
int  get_tab_count();
int  get_active_tab();

std::string              get_line_text(int line);
std::vector<std::string> get_open_files();
std::string              get_tab_file(int index);

// ---------- Mutate (script → host) ----------

void set_cursor_pos(int line, int col);
void set_selection (int start_line, int start_col, int end_line, int end_col);

void insert_text      (const std::string& text);
void replace_selection(const std::string& text);

void open_file        (const std::string& path);
void save_active_file ();
void goto_line        (int line);

// User-facing signals
void show_notification(const std::string& msg);
void set_status       (const std::string& msg);
void send_chat_message(const std::string& msg);

// ---------- Events fired by the host ----------

// Each event carries the file path (and, where relevant, additional data).
// Extensions subscribe via callbacks::add or the higher-level `on_*` hooks
// registered by name in their script.

void on_file_opened    (const std::string& path);
void on_file_saved     (const std::string& path);
void on_buffer_changed (const std::string& path, int line);
void on_tab_changed    (const std::string& path);

// ---------- Backend wiring ----------

struct Backend {
    // Query
    std::function<std::string()>                  get_active_file;
    std::function<std::string()>                  get_active_file_content;
    std::function<std::string()>                  get_active_language;
    std::function<std::string()>                  get_root_path;
    std::function<std::string()>                  get_selection_text;
    std::function<int()>                          get_cursor_line;
    std::function<int()>                          get_cursor_col;
    std::function<int()>                          get_line_count;
    std::function<int()>                          get_tab_count;
    std::function<int()>                          get_active_tab;
    std::function<std::string(int)>               get_line_text;
    std::function<std::vector<std::string>()>     get_open_files;
    std::function<std::string(int)>               get_tab_file;

    // Mutate
    std::function<void(int, int)>                 set_cursor_pos;
    std::function<void(int, int, int, int)>       set_selection;
    std::function<void(std::string)>              insert_text;
    std::function<void(std::string)>              replace_selection;
    std::function<void(std::string)>              open_file;
    std::function<void()>                         save_active_file;
    std::function<void(int)>                      goto_line;
    std::function<void(std::string)>              show_notification;
    std::function<void(std::string)>              set_status;
    std::function<void(std::string)>              send_chat_message;
};

void install(Backend b);

} // namespace paris::editor
