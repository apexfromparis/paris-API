#include "paris/engine.hpp"

#include <cstdio>
#include <mutex>
#include <string>

namespace paris {

namespace {
    std::mutex  g_mtx;
    log_fn_t    g_logger    = nullptr;
    std::string g_user_name = "anonymous";

    void default_log(LogLevel lvl, LogSink sink, const std::string& msg) {
        const char* lvl_tag  = "info";
        switch (lvl) {
            case LogLevel::Info:  lvl_tag = "info";  break;
            case LogLevel::Warn:  lvl_tag = "warn";  break;
            case LogLevel::Error: lvl_tag = "error"; break;
        }
        const char* sink_tag = sink == LogSink::Overlay ? "ui" : "console";
        std::fprintf(stderr, "[paris/%s/%s] %s\n", lvl_tag, sink_tag, msg.c_str());
    }
}

void set_logger(log_fn_t fn) { std::lock_guard lk(g_mtx); g_logger = fn; }

void set_user_name(const std::string& n) { std::lock_guard lk(g_mtx); g_user_name = n; }
std::string get_user_name()              { std::lock_guard lk(g_mtx); return g_user_name; }

namespace {
    void dispatch(LogLevel lvl, LogSink sink, const std::string& msg) {
        log_fn_t fn;
        { std::lock_guard lk(g_mtx); fn = g_logger; }
        (fn ? fn : default_log)(lvl, sink, msg);
    }
}

void log              (const std::string& m) { dispatch(LogLevel::Info,  LogSink::Overlay, m); }
void log_error        (const std::string& m) { dispatch(LogLevel::Error, LogSink::Overlay, m); }
void log_console      (const std::string& m) { dispatch(LogLevel::Info,  LogSink::Console, m); }
void log_console_error(const std::string& m) { dispatch(LogLevel::Error, LogSink::Console, m); }

} // namespace paris
