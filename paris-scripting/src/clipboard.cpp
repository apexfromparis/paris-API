#include "paris/clipboard.hpp"

#include <mutex>

namespace paris::clipboard {

namespace {
    std::mutex g_mtx;
    Backend    g_backend;
}

void install(Backend b) {
    std::lock_guard lk(g_mtx);
    g_backend = std::move(b);
}

std::string get() {
    std::lock_guard lk(g_mtx);
    return g_backend.get ? g_backend.get() : std::string{};
}

void set(const std::string& t) {
    std::lock_guard lk(g_mtx);
    if (g_backend.set) g_backend.set(t);
}

} // namespace paris::clipboard
