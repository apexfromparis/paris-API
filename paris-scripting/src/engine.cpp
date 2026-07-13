#include "paris/engine.hpp"

#include "paris/ai.hpp"
#include "paris/callbacks.hpp"
#include "paris/intellisense.hpp"
#include "paris/settings.hpp"
#include "paris/tools.hpp"
#include "paris/ui.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_map>

namespace paris {

std::unique_ptr<Script> make_lua_script(std::string name);
std::unique_ptr<Script> make_as_script (std::string name);
void                    shutdown_as_engine();

struct ScriptEngine::Impl {
    std::mutex mtx;
    std::unordered_map<std::string, std::unique_ptr<Script>> scripts;
};

ScriptEngine::ScriptEngine()  : impl_(std::make_unique<Impl>()) {}
ScriptEngine::~ScriptEngine() { shutdown(); shutdown_as_engine(); }

namespace {
    std::string read_file(const std::filesystem::path& p) {
        std::ifstream f(p, std::ios::binary);
        if (!f) return {};
        std::stringstream ss; ss << f.rdbuf();
        return ss.str();
    }
}

bool ScriptEngine::load_file(const std::filesystem::path& file) {
    auto ext  = file.extension().string();
    auto name = file.stem().string();

    std::unique_ptr<Script> s;
    if      (ext == ".lua") s = make_lua_script(name);
    else if (ext == ".as")  s = make_as_script(name);
    else {
        log_console("skip (unknown extension): " + file.string());
        return false;
    }

    auto src = read_file(file);
    if (src.empty()) {
        log_console("skip (empty or unreadable): " + file.string());
        return false;
    }

    bool persist = s->load(src);
    if (!persist) {
        ui::drop_section(name);
        callbacks::drop_section(name);
        intellisense::drop_section(name);
        ai::drop_section(name);
        tools::drop_section(name);
        settings::drop_section(name);
        log_console("script '" + name + "' returned <= 0 from main, dropped");
        return false;
    }

    std::lock_guard lk(impl_->mtx);
    auto it = impl_->scripts.find(name);
    if (it != impl_->scripts.end()) {
        // Reloading — run the old one's on_unload and drop its UI/callbacks.
        it->second->unload_notify();
        ui::drop_section(name);
        callbacks::drop_section(name);
        intellisense::drop_section(name);
        ai::drop_section(name);
        tools::drop_section(name);
        settings::drop_section(name);
        impl_->scripts.erase(it);
    }
    log_console("loaded " + s->language() + " script '" + name + "'");
    impl_->scripts.emplace(name, std::move(s));
    return true;
}

void ScriptEngine::load_directory(const std::filesystem::path& dir) {
    if (!std::filesystem::exists(dir)) {
        log_console("scripts dir missing: " + dir.string());
        return;
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        if (ext == ".lua" || ext == ".as") load_file(entry.path());
    }
}

void ScriptEngine::tick() {
    // Fire the plain 'on_frame' event first (any script or host component
    // that registered a callback via `callbacks::add`).
    callbacks::fire("on_frame");

    // Then poll each managed script's own tick() — this is what runs Lua's
    // on_frame or AngelScript's void on_frame(). If any returns false, it's
    // asking to be unloaded.
    std::vector<std::string> dead;
    {
        std::lock_guard lk(impl_->mtx);
        for (auto& [name, s] : impl_->scripts) {
            if (!s->tick()) dead.push_back(name);
        }
    }
    for (auto& n : dead) unload(n);
}

void ScriptEngine::shutdown() {
    std::lock_guard lk(impl_->mtx);
    for (auto& [name, s] : impl_->scripts) {
        s->unload_notify();
        ui::drop_section(name);
        callbacks::drop_section(name);
        intellisense::drop_section(name);
        ai::drop_section(name);
        tools::drop_section(name);
        settings::drop_section(name);
    }
    // Global on_unload for any host-registered callbacks (not per-script).
    callbacks::fire("on_unload");
    impl_->scripts.clear();
}

void ScriptEngine::unload(const std::string& n) {
    std::lock_guard lk(impl_->mtx);
    auto it = impl_->scripts.find(n);
    if (it == impl_->scripts.end()) return;
    it->second->unload_notify();
    ui::drop_section(n);
    callbacks::drop_section(n);
    intellisense::drop_section(n);
    ai::drop_section(n);
    tools::drop_section(n);
    settings::drop_section(n);
    impl_->scripts.erase(it);
    log_console("unloaded '" + n + "'");
}

std::vector<ScriptEngine::ScriptInfo> ScriptEngine::list() const {
    std::lock_guard lk(impl_->mtx);
    std::vector<ScriptInfo> out;
    out.reserve(impl_->scripts.size());
    for (const auto& [name, s] : impl_->scripts) out.push_back({ name, s->language() });
    return out;
}

} // namespace paris
