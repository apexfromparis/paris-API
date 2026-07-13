#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace paris {

class ScriptEngine;

// Watches a directory and reloads any .lua / .as file that changes on disk.
// Runs a background thread — the engine callback is fired from that thread,
// so the ScriptEngine's own mutex protects concurrent access.
class HotReload {
public:
    explicit HotReload(ScriptEngine& engine);
    ~HotReload();

    // Non-recursive by default. Set recursive to also watch subdirectories.
    void start(const std::filesystem::path& dir, bool recursive = false);
    void stop();

    bool is_running() const;

    // Optional — swap the default (which calls engine.load_file) for
    // logging, throttling, or filtering.
    void set_on_change(std::function<void(const std::filesystem::path&)> fn);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace paris
