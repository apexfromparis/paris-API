#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace paris {

// ---------- Logging (mirrors perception.cx's `engine.log*`) ----------

enum class LogLevel { Info, Warn, Error };
enum class LogSink  {
    Overlay,       // top-left fading notification
    Console        // persistent, no auto-fade
};

using log_fn_t = void(*)(LogLevel, LogSink, const std::string&);

void set_logger(log_fn_t fn);
void set_user_name(const std::string& name);
std::string get_user_name();

void log              (const std::string& msg);
void log_error        (const std::string& msg);
void log_console      (const std::string& msg);
void log_console_error(const std::string& msg);

// ---------- Script + engine ----------

// One loaded script, regardless of language.
class Script {
public:
    virtual ~Script() = default;
    // Runs the script's initializer. Returns true if the script should persist.
    virtual bool  load(const std::string& source) = 0;
    // Runs the per-frame hook. Returns false if the script asked to unload.
    virtual bool  tick() = 0;
    // Runs the on-unload hook once.
    virtual void  unload_notify() = 0;

    virtual const std::string& name()     const = 0;
    virtual const std::string& language() const = 0;
};

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    // Discover, load, and initialise every .lua / .as file under `dir`.
    void load_directory(const std::filesystem::path& dir);

    // Load a single file. Returns false on error.
    bool load_file(const std::filesystem::path& file);

    // Call once per frame. Runs each surviving script's on_frame and drops
    // any that ask to unload.
    void tick();

    // Call once at program exit. Runs on_unload for each script, then drops.
    void shutdown();

    // Force a single script off.
    void unload(const std::string& script_name);

    struct ScriptInfo {
        std::string name;
        std::string language;
    };
    std::vector<ScriptInfo> list() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace paris
