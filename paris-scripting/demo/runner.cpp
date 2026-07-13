// Multi-game Lua script runner.
//
// Loads every .lua under a directory and executes them against the paris
// scripting engine. A game_profile is picked based on the current process
// name (via the registered matchers); if no profile matches, scripts still
// load — game-side queries just return empty until you wire a backend.
//
// Usage: paris_runner <scripts_dir>

#include "paris/engine.hpp"
#include "paris/game_profile.hpp"
#include "paris/input.hpp"
#include "paris/render.hpp"
#include "paris/sandbox.hpp"
#include "paris/ui.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

namespace {

std::string current_process_name() {
#ifdef _WIN32
    char buf[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    return p.filename().string();
#else
    return "unknown";
#endif
}

// Install a headless render backend that prints text draws. Replace this in
// your real host — it's here so the runner shows something without a
// dependency on a GPU stack.
void install_headless_render() {
    paris::render::Backend b;
    b.get_view       = []() { return paris::vector2{ 1920.f, 1080.f }; };
    b.get_view_scale = []() { return 1.f; };
    b.get_fps        = []() { return 144.f; };
    b.draw_text = [](paris::vector2 p, paris::color_t c, std::string s,
                     paris::render::font_id_t f, paris::render::TextEffect,
                     paris::color_t) {
        std::printf("text  (%.0f, %.0f) font=%d rgba(%d,%d,%d,%d) '%s'\n",
                    p.x, p.y, f, c.r, c.g, c.b, c.a, s.c_str());
    };
    b.draw_rect_filled = [](paris::vector2 a, paris::vector2 b_, paris::color_t c, float) {
        std::printf("fill  (%.0f, %.0f)-(%.0f, %.0f) rgba(%d,%d,%d,%d)\n",
                    a.x, a.y, b_.x, b_.y, c.r, c.g, c.b, c.a);
    };
    paris::render::install(std::move(b));
}

// Register game-profile matchers. Each pair is (process substring, profile
// name). The runner picks the first match; add your own here.
void register_matchers() {
    paris::game::register_profile_matcher("cs2.exe",  "cs2");
    paris::game::register_profile_matcher("csgo.exe", "csgo");
}

} // namespace

int main(int argc, char** argv) {
    std::string dir = argc > 1 ? argv[1] : "scripts";

    paris::set_user_name("runner");
    install_headless_render();
    register_matchers();

    paris::sandbox::set_frame_budget_ms(8);

    auto proc = current_process_name();
    auto profile = paris::game::autodetect_profile(proc);
    if (!profile.empty()) {
        std::printf("[runner] game profile: %s\n", profile.c_str());
    } else {
        std::printf("[runner] no profile matched for '%s' — scripts run against empty game backend\n",
                    proc.c_str());
    }

    paris::ScriptEngine engine;
    paris::ui::set_menu_visible(true);
    engine.load_directory(dir);

    auto list = engine.list();
    std::printf("[runner] loaded %zu script(s):\n", list.size());
    for (auto& s : list) std::printf("  - %s (%s)\n", s.name.c_str(), s.language.c_str());

    // Fake input snapshot so per-frame input reads see a stable state.
    paris::input::FrameSnapshot snap;

    for (int i = 0; i < 5; ++i) {
        std::printf("--- frame %d ---\n", i);
        paris::input::begin_frame(snap);

        // Fire create_move so scripts subscribed to that event see a UserCmd.
        paris::game::UserCmd cmd;
        paris::game::fire_create_move(cmd);

        engine.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    engine.shutdown();
    return 0;
}
