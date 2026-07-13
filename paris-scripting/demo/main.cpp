// Minimal host. In a real usermode you'd have ImGui + your game's renderer;
// here everything shim-prints to stdout so you can see the pipeline run.
//
// Showcases: sandbox timeout, hot reload, keybind driver, autosave.

#include "paris/autosave.hpp"
#include "paris/engine.hpp"
#include "paris/hot_reload.hpp"
#include "paris/input.hpp"
#include "paris/keybind_driver.hpp"
#include "paris/render.hpp"
#include "paris/sandbox.hpp"
#include "paris/ui.hpp"

#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    paris::set_user_name("clg-demo");

    // ---- render backend ----
    paris::render::Backend b;
    b.get_view       = []()  { return paris::vector2{ 1920.f, 1080.f }; };
    b.get_view_scale = []()  { return 1.f; };
    b.get_fps        = []()  { return 144.f; };
    b.draw_rect = [](paris::vector2 mn, paris::vector2 mx, paris::color_t c, float t, float r) {
        std::printf("rect  (%.0f,%.0f)-(%.0f,%.0f) rgba(%d,%d,%d,%d) t=%.1f r=%.1f\n",
                    mn.x, mn.y, mx.x, mx.y, c.r, c.g, c.b, c.a, t, r);
    };
    b.draw_rect_filled = [](paris::vector2 mn, paris::vector2 mx, paris::color_t c, float r) {
        std::printf("fill  (%.0f,%.0f)-(%.0f,%.0f) rgba(%d,%d,%d,%d) r=%.1f\n",
                    mn.x, mn.y, mx.x, mx.y, c.r, c.g, c.b, c.a, r);
    };
    b.draw_text = [](paris::vector2 p, paris::color_t c, std::string s,
                     paris::render::font_id_t f, paris::render::TextEffect fx,
                     paris::color_t /*fxc*/) {
        std::printf("text  (%.0f,%.0f) font=%d fx=%d rgba(%d,%d,%d,%d) '%s'\n",
                    p.x, p.y, f, int(fx), c.r, c.g, c.b, c.a, s.c_str());
    };
    b.get_text_size = [](std::string s, paris::render::font_id_t f) {
        return paris::render::TextSize{ float(s.size() * f) * 0.5f, float(f) };
    };
    b.world_to_screen = [](paris::vector3 w, paris::vector2& out) {
        if (w.z >= 0.f) return false;
        out = { 960.f + w.x * 200.f / -w.z, 540.f + w.y * 200.f / -w.z };
        return true;
    };
    paris::render::install(std::move(b));

    // ---- sandbox: give scripts up to 4ms per hook. Zero disables. ----
    paris::sandbox::set_frame_budget_ms(4);

    // ---- engine + scripts ----
    paris::ScriptEngine engine;
    paris::ui::set_menu_visible(true);
    engine.load_directory("scripts");

    // ---- autosave: load previous state, save on shutdown ----
    paris::autosave::install_hooks();

    // ---- hot reload: save a .lua/.as file, it reloads ----
    paris::HotReload watcher(engine);
    watcher.start("scripts");

    // ---- fake input snapshot (no keys held) ----
    paris::input::FrameSnapshot snap;

    for (int i = 0; i < 3; ++i) {
        std::printf("--- frame %d ---\n", i);
        paris::input::begin_frame(snap);
        paris::keybind_driver::tick();
        engine.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    watcher.stop();
    engine.shutdown();
    return 0;
}
