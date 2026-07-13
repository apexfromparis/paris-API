#pragma once

#include <functional>
#include <string>

// Access to the IDE's integrated terminal. Extensions can spawn tabs, write
// input, and read output. The host implements the actual PTY plumbing.

namespace paris::terminal {

using terminal_id_t = int;

terminal_id_t create      (const std::string& title = "terminal");
void          send_text   (terminal_id_t id, const std::string& text);
void          send_command(terminal_id_t id, const std::string& command); // convenience: send_text + "\n"
std::string   read_output (terminal_id_t id, int max_bytes = 4096);
void          clear       (terminal_id_t id);
void          close       (terminal_id_t id);
void          set_active  (terminal_id_t id);
terminal_id_t get_active  ();
int           count       ();

struct Backend {
    std::function<terminal_id_t(std::string)>            create;
    std::function<void(terminal_id_t, std::string)>      send_text;
    std::function<std::string(terminal_id_t, int)>       read_output;
    std::function<void(terminal_id_t)>                   clear;
    std::function<void(terminal_id_t)>                   close;
    std::function<void(terminal_id_t)>                   set_active;
    std::function<terminal_id_t()>                       get_active;
    std::function<int()>                                 count;
};

void install(Backend b);

} // namespace paris::terminal
