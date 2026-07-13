#include "paris/terminal.hpp"

#include <mutex>

namespace paris::terminal {

namespace {
    std::mutex g_mtx;
    Backend    g_backend;
}

void install(Backend b) { std::lock_guard lk(g_mtx); g_backend = std::move(b); }

terminal_id_t create(const std::string& title) {
    std::lock_guard lk(g_mtx);
    return g_backend.create ? g_backend.create(title) : 0;
}

void send_text(terminal_id_t id, const std::string& t) {
    std::lock_guard lk(g_mtx);
    if (g_backend.send_text) g_backend.send_text(id, t);
}

void send_command(terminal_id_t id, const std::string& cmd) {
    send_text(id, cmd + "\n");
}

std::string read_output(terminal_id_t id, int n) {
    std::lock_guard lk(g_mtx);
    return g_backend.read_output ? g_backend.read_output(id, n) : std::string{};
}

void clear(terminal_id_t id) {
    std::lock_guard lk(g_mtx);
    if (g_backend.clear) g_backend.clear(id);
}

void close(terminal_id_t id) {
    std::lock_guard lk(g_mtx);
    if (g_backend.close) g_backend.close(id);
}

void set_active(terminal_id_t id) {
    std::lock_guard lk(g_mtx);
    if (g_backend.set_active) g_backend.set_active(id);
}

terminal_id_t get_active() {
    std::lock_guard lk(g_mtx);
    return g_backend.get_active ? g_backend.get_active() : 0;
}

int count() {
    std::lock_guard lk(g_mtx);
    return g_backend.count ? g_backend.count() : 0;
}

} // namespace paris::terminal
