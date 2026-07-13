// Lua backend via sol2. Exposes every module in `paris::` to scripts in the
// naming style perception.cx documents: `ui.*`, `render.*`, `input.*`, `fs.*`,
// `mathx.*`, `util.*`, `json.*`, `net.*`, `engine.*`, plus lifecycle hooks
// `main()` / `on_frame()` / `on_unload()`.

#include "paris/ai.hpp"
#include "paris/callbacks.hpp"
#include "paris/clipboard.hpp"
#include "paris/editor.hpp"
#include "paris/engine.hpp"
#include "paris/fs.hpp"
#include "paris/game_profile.hpp"
#include "paris/input.hpp"
#include "paris/intellisense.hpp"
#include "paris/json.hpp"
#include "paris/mathx.hpp"
#include "paris/net.hpp"
#include "paris/paris_compat.hpp"
#include "paris/render.hpp"
#include "paris/sandbox.hpp"
#include "paris/settings.hpp"
#include "paris/terminal.hpp"
#include "paris/tools.hpp"
#include "paris/types.hpp"
#include "paris/ui.hpp"
#include "paris/util.hpp"

#include <any>

#include <sol/sol.hpp>
#include <lua.hpp>

#include <memory>
#include <string>
#include <utility>

namespace paris {

class LuaScript final : public Script {
public:
    LuaScript(std::string name) : name_(std::move(name)) {
        lua_.open_libraries(
            sol::lib::base, sol::lib::math, sol::lib::string,
            sol::lib::table, sol::lib::os,   sol::lib::coroutine,
            sol::lib::utf8);

        // Sandbox: fire the timeout hook every N instructions. The hook raises
        // a Lua error when the current frame budget is blown, which unwinds
        // the pcall stack and returns cleanly to the C++ tick().
        lua_sethook(lua_.lua_state(), &LuaScript::timeout_hook_c,
                    LUA_MASKCOUNT, sandbox::get_lua_instruction_step());

        bind_types();
        bind_ui();
        bind_render();
        bind_input();
        bind_fs();
        bind_mathx();
        bind_util();
        bind_json();
        bind_net();
        bind_engine();
        bind_callbacks();
        bind_editor();
        bind_clipboard();
        bind_intellisense();
        bind_ai();
        bind_tools();
        bind_settings();
        bind_terminal();
        redirect_print();

        // Compat shim for paris / gamesense flavour scripts (gui.ctx:find,
        // events.<name>:add, draw.*, entities.*, game.*, hvh, theme, mods,
        // ant, utils, ffi stub, math.calc_angle / math.vec3 / math.vec2).
        // Installed after our own bindings so it can wrap them.
        paris_compat::install(lua_);
    }

    static void timeout_hook_c(lua_State* L, lua_Debug*) {
        if (sandbox::over_budget())
            luaL_error(L, "paris: script exceeded frame budget");
    }

    bool load(const std::string& source) override {
        callbacks::detail::push_section(name_);
        ui::detail::push_section(name_);
        sandbox::begin_hook();

        auto result = lua_.safe_script(source,
            [](lua_State*, sol::protected_function_result r) { return r; });

        if (!result.valid()) {
            sol::error err = result;
            log_console_error("lua load [" + name_ + "]: " + err.what());
            callbacks::detail::pop_section();
            ui::detail::pop_section();
            return false;
        }

        // Overlay-script convention: `main()` returns int, persists when > 0
        // and `on_frame` exists. IDE-extension convention: `on_activate()`
        // and any of the `on_*` hooks. We support both — whichever the script
        // defines drives the lifecycle.
        int rc = 1;

        sol::protected_function activate_fn = lua_["on_activate"];
        if (activate_fn.valid()) {
            sol::protected_function_result r = activate_fn();
            if (!r.valid()) {
                sol::error err = r;
                log_console_error("lua on_activate [" + name_ + "]: " + err.what());
                callbacks::detail::pop_section();
                ui::detail::pop_section();
                return false;
            }
        } else {
            sol::protected_function main_fn = lua_["main"];
            if (main_fn.valid()) {
                sol::protected_function_result mres = main_fn();
                if (!mres.valid()) {
                    sol::error err = mres;
                    log_console_error("lua main() [" + name_ + "]: " + err.what());
                    callbacks::detail::pop_section();
                    ui::detail::pop_section();
                    return false;
                }
                if (mres.get_type(0) == sol::type::number) rc = mres.get<int>(0);
            }
        }

        // Auto-wire IDE event hooks. The script defines any subset of these
        // as top-level functions; we register them as typed callbacks so the
        // host can fire them by event name and drop them cleanly on unload.
        wire_ide_hooks();

        callbacks::detail::pop_section();
        ui::detail::pop_section();

        on_frame_      = lua_["on_frame"];
        on_tick_       = lua_["on_tick"];
        on_unload_     = lua_["on_unload"];
        on_deactivate_ = lua_["on_deactivate"];

        // Persist when any of the frame-loop hooks exist, or when the script
        // registered any event hook / provider. Passive IDE extensions with
        // only event handlers still need to stay resident.
        bool has_frame_hook = on_frame_.valid() || on_tick_.valid();
        return has_frame_hook || activate_fn.valid();
    }

    bool tick() override {
        // Prefer `on_tick` (IDE convention) over `on_frame` (overlay convention).
        auto& fn = on_tick_.valid() ? on_tick_ : on_frame_;
        if (!fn.valid()) return true;
        callbacks::detail::push_section(name_);
        sandbox::begin_hook();
        auto r = fn();
        callbacks::detail::pop_section();
        if (!r.valid()) {
            sol::error err = r;
            log_console_error("lua tick [" + name_ + "]: " + err.what());
            return false;
        }
        if (r.get_type(0) == sol::type::number) return r.get<int>(0) > 0;
        return true;
    }

    void unload_notify() override {
        auto call = [this](sol::protected_function& f, const char* label) {
            if (!f.valid()) return;
            callbacks::detail::push_section(name_);
            auto r = f();
            callbacks::detail::pop_section();
            if (!r.valid()) {
                sol::error err = r;
                log_console_error(std::string("lua ") + label + " [" + name_ + "]: " + err.what());
            }
        };
        call(on_deactivate_, "on_deactivate");
        call(on_unload_,     "on_unload");

        // Drop any IDE-scoped registrations owned by this script.
        intellisense::drop_section(name_);
        ai::drop_section(name_);
        tools::drop_section(name_);
        settings::drop_section(name_);
    }

    const std::string& name()     const override { return name_; }
    const std::string& language() const override { static const std::string s = "lua"; return s; }

private:
    // ---------- bindings ----------

    void bind_types() {
        lua_.new_usertype<vector2>("vector2",
            "new", sol::factories(
                []()               { return vector2{}; },
                [](float x, float y){ return vector2{x, y}; }),
            "x", &vector2::x, "y", &vector2::y,
            "length",     &vector2::length,
            "length_sqr", &vector2::length_sqr,
            "distance",   &vector2::distance,
            "dot",        &vector2::dot,
            "normalized", &vector2::normalized,
            "to_string",  &vector2::to_string,
            sol::meta_function::addition,       &vector2::operator+,
            sol::meta_function::subtraction,    &vector2::operator-,
            sol::meta_function::multiplication, &vector2::operator*);
        lua_["vec2_t"] = lua_["vector2"];

        lua_.new_usertype<vector3>("vector3",
            "new", sol::factories(
                []()                          { return vector3{}; },
                [](float x, float y, float z) { return vector3{x, y, z}; }),
            "x", &vector3::x, "y", &vector3::y, "z", &vector3::z,
            "length",     &vector3::length,
            "length_sqr", &vector3::length_sqr,
            "distance",   &vector3::distance,
            "dot",        &vector3::dot,
            "cross",      &vector3::cross,
            "normalized", &vector3::normalized,
            "to_screen",  [this](const vector3& v) -> sol::object {
                vector2 out;
                if (!v.to_screen(out)) return sol::lua_nil;
                return sol::make_object(lua_.lua_state(), out);
            },
            "to_string",  &vector3::to_string,
            sol::meta_function::addition,       &vector3::operator+,
            sol::meta_function::subtraction,    &vector3::operator-,
            sol::meta_function::multiplication, &vector3::operator*);
        lua_["vec3_t"] = lua_["vector3"];

        lua_.new_usertype<color_t>("color_t",
            "new", sol::factories(
                [](int r, int g, int b)        { return color_t{r,g,b}; },
                [](int r, int g, int b, int a) { return color_t{r,g,b,a}; }),
            "r", &color_t::r, "g", &color_t::g, "b", &color_t::b, "a", &color_t::a,
            "with_alpha", &color_t::with_alpha,
            "lerp",       &color_t::lerp);
        lua_["color_t"]["from_hsv"] = &color_t::from_hsv;

        lua_.new_usertype<quaternion>("quaternion",
            "new", sol::factories(
                []()                                  { return quaternion::identity(); },
                [](float x, float y, float z, float w){ return quaternion{x,y,z,w}; }),
            "x", &quaternion::x, "y", &quaternion::y, "z", &quaternion::z, "w", &quaternion::w,
            "rotate",  &quaternion::rotate,
            "inverse", &quaternion::inverse,
            "to_euler", &quaternion::to_euler,
            sol::meta_function::multiplication, &quaternion::operator*);
        lua_["quaternion"]["identity"]   = &quaternion::identity;
        lua_["quaternion"]["from_euler"] = &quaternion::from_euler;

        lua_.new_usertype<matrix4x4>("matrix4x4",
            "new", sol::factories([]() { return matrix4x4::identity(); }),
            "at",         sol::overload(
                [](matrix4x4& m, int r, int c)                { return m.at(r, c); },
                [](matrix4x4& m, int r, int c, float v)       { m.at(r, c) = v; }),
            "transform",  &matrix4x4::transform,
            sol::meta_function::multiplication, &matrix4x4::operator*);
    }

    void bind_ui() {
        auto ui = lua_["ui"].get_or_create<sol::table>();

        ui.new_usertype<ui::Checkbox>("Checkbox",
            "get", &ui::Checkbox::get,
            "set", &ui::Checkbox::set,
            "set_active", &ui::Checkbox::set_active);
        ui.new_usertype<ui::SliderInt>("SliderInt",
            "get", &ui::SliderInt::get,
            "set", &ui::SliderInt::set,
            "set_active", &ui::SliderInt::set_active);
        ui.new_usertype<ui::SliderDouble>("SliderDouble",
            "get", &ui::SliderDouble::get,
            "set", &ui::SliderDouble::set,
            "set_active", &ui::SliderDouble::set_active);
        ui.new_usertype<ui::Input>("Input",
            "get", &ui::Input::get,
            "set", &ui::Input::set,
            "set_active", &ui::Input::set_active);
        ui.new_usertype<ui::MultiSelect>("MultiSelect",
            "get", &ui::MultiSelect::get,
            "set", &ui::MultiSelect::set,
            "set_active", &ui::MultiSelect::set_active);
        ui.new_usertype<ui::SingleSelect>("SingleSelect",
            "get", &ui::SingleSelect::get,
            "set", &ui::SingleSelect::set,
            "set_active", &ui::SingleSelect::set_active);
        ui.new_usertype<ui::Keybind>("Keybind",
            "is_pressed", &ui::Keybind::is_pressed,
            "set_active", &ui::Keybind::set_active,
            "set", [](ui::Keybind& kb, int vk, int mode) {
                kb.set(vk, static_cast<ui::KeybindMode>(mode));
            });
        ui.new_usertype<ui::ColorPicker>("ColorPicker",
            "get", &ui::ColorPicker::get,
            "set", &ui::ColorPicker::set,
            "set_active", &ui::ColorPicker::set_active);
        ui.new_usertype<ui::Button>("Button",
            "set_active", &ui::Button::set_active);
        ui.new_usertype<ui::List>("List",
            "append",           &ui::List::append,
            "append_after",     &ui::List::append_after,
            "get",              &ui::List::get,
            "get_all",          &ui::List::get_all,
            "get_count",        &ui::List::get_count,
            "clear",            &ui::List::clear,
            "highlight",        &ui::List::highlight,
            "remove_highlight", &ui::List::remove_highlight,
            "remove",           &ui::List::remove,
            "set_active_index", &ui::List::set_active_index,
            "set_active",       &ui::List::set_active);

        ui.new_usertype<ui::Panel>("Panel",
            "add_checkbox",      &ui::Panel::add_checkbox,
            "add_slider_int",    &ui::Panel::add_slider_int,
            "add_slider_double", &ui::Panel::add_slider_double,
            "add_input",         &ui::Panel::add_input,
            "add_multi_select",  [](ui::Panel& p, const std::string& n,
                                    sol::table opts, bool exp,
                                    sol::optional<bool> dt, sol::optional<bool> fp) {
                std::vector<std::string> v;
                for (auto& kv : opts) v.push_back(kv.second.as<std::string>());
                return p.add_multi_select(n, std::move(v), exp, dt.value_or(true), fp.value_or(false));
            },
            "add_single_select", [](ui::Panel& p, const std::string& n,
                                    sol::table opts, int init, bool exp,
                                    sol::optional<bool> dt, sol::optional<bool> fp) {
                std::vector<std::string> v;
                for (auto& kv : opts) v.push_back(kv.second.as<std::string>());
                return p.add_single_select(n, std::move(v), init, exp, dt.value_or(true), fp.value_or(false));
            },
            "add_keybind",       [](ui::Panel& p, ui::Checkbox parent, const std::string& n,
                                    int vk, const std::string& mode,
                                    sol::optional<bool> dt, sol::optional<bool> fp) {
                ui::KeybindMode m = ui::KeybindMode::Off;
                if      (mode == "on")        m = ui::KeybindMode::On;
                else if (mode == "single")    m = ui::KeybindMode::Single;
                else if (mode == "toggle")    m = ui::KeybindMode::Toggle;
                else if (mode == "always_on") m = ui::KeybindMode::AlwaysOn;
                return p.add_keybind(parent, n, vk, m, dt.value_or(true), fp.value_or(false));
            },
            "add_color",         [](ui::Panel& p, ui::Checkbox parent, const std::string& n,
                                    sol::table rgba, sol::optional<bool> fp) {
                color_t c{ rgba[1], rgba[2], rgba[3], rgba.get_or<int>(4, 255) };
                return p.add_color(parent, n, c, fp.value_or(false));
            },
            "add_button",        [name = name_](ui::Panel& p, const std::string& n, sol::protected_function fn) {
                auto shared = std::make_shared<sol::protected_function>(std::move(fn));
                return p.add_button(n, [shared, name]() {
                    auto r = (*shared)();
                    if (!r.valid()) {
                        sol::error err = r;
                        log_console_error("lua button [" + name + "] " + err.what());
                    }
                });
            },
            "add_list",          [](ui::Panel& p, const std::string& n, sol::table rows) {
                std::vector<ui::ListRow> v;
                for (auto& kv : rows) {
                    sol::table row = kv.second;
                    v.push_back({ row.get_or<std::string>("name", ""),
                                  row.get_or<std::string>("info", ""), false });
                }
                return p.add_list(n, std::move(v));
            });

        ui.new_usertype<ui::Subtab>("Subtab",
            "add_panel", &ui::Subtab::add_panel,
            "set_active", &ui::Subtab::set_active);

        ui.set_function("create_subtab", &ui::create_subtab);
        ui.set_function("find_element",  &ui::find_element);
        ui.set_function("is_active",     &ui::is_active);
        ui.set_function("construct_config", &ui::construct_config);
        ui.set_function("apply_config",     &ui::apply_config);
    }

    void bind_render() {
        auto r = lua_["render"].get_or_create<sol::table>();

        r["FONT_18"] = int(render::kFont18);
        r["FONT_20"] = int(render::kFont20);
        r["FONT_24"] = int(render::kFont24);
        r["FONT_28"] = int(render::kFont28);

        r["EFFECT_NONE"]    = int(render::TextEffect::None);
        r["EFFECT_OUTLINE"] = int(render::TextEffect::Outline);
        r["EFFECT_SHADOW"]  = int(render::TextEffect::Shadow);
        r["EFFECT_GLOW"]    = int(render::TextEffect::Glow);

        r.set_function("get_view",       &render::get_view);
        r.set_function("get_view_scale", &render::get_view_scale);
        r.set_function("get_fps",        &render::get_fps);

        r.set_function("draw_rect", sol::overload(
            [](vector2 a, vector2 b, color_t c)                            { render::draw_rect(a, b, c, 1.f, 0.f); },
            [](vector2 a, vector2 b, color_t c, float t)                   { render::draw_rect(a, b, c, t, 0.f); },
            [](vector2 a, vector2 b, color_t c, float t, float rnd)        { render::draw_rect(a, b, c, t, rnd); }));

        r.set_function("draw_rect_filled", sol::overload(
            [](vector2 a, vector2 b, color_t c)               { render::draw_rect_filled(a, b, c, 0.f); },
            [](vector2 a, vector2 b, color_t c, float rnd)    { render::draw_rect_filled(a, b, c, rnd); }));

        r.set_function("draw_gradient", &render::draw_gradient);

        r.set_function("draw_line", sol::overload(
            [](vector2 a, vector2 b, color_t c)               { render::draw_line(a, b, c, 1.f); },
            [](vector2 a, vector2 b, color_t c, float t)      { render::draw_line(a, b, c, t); }));

        r.set_function("draw_circle", sol::overload(
            [](vector2 c, float r_, color_t col)              { render::draw_circle(c, r_, col, 1.f, 24); },
            [](vector2 c, float r_, color_t col, float t)     { render::draw_circle(c, r_, col, t, 24); },
            [](vector2 c, float r_, color_t col, float t, int seg){ render::draw_circle(c, r_, col, t, seg); }));

        r.set_function("draw_circle_filled", sol::overload(
            [](vector2 c, float r_, color_t col)              { render::draw_circle_filled(c, r_, col, 24); },
            [](vector2 c, float r_, color_t col, int seg)     { render::draw_circle_filled(c, r_, col, seg); }));

        r.set_function("draw_arc", &render::draw_arc);
        r.set_function("draw_triangle", &render::draw_triangle);
        r.set_function("draw_triangle_filled", &render::draw_triangle_filled);

        r.set_function("draw_polygon", [](sol::table pts, color_t c, sol::optional<float> t) {
            std::vector<vector2> v;
            for (auto& kv : pts) v.push_back(kv.second.as<vector2>());
            render::draw_polygon(v, c, t.value_or(1.f));
        });
        r.set_function("draw_polygon_filled", [](sol::table pts, color_t c) {
            std::vector<vector2> v;
            for (auto& kv : pts) v.push_back(kv.second.as<vector2>());
            render::draw_polygon_filled(v, c);
        });

        r.set_function("draw_text", sol::overload(
            [](vector2 p, color_t c, std::string s) {
                render::draw_text(p, c, s, render::kFont20, render::TextEffect::None, { 0,0,0,200 });
            },
            [](vector2 p, color_t c, std::string s, int font) {
                render::draw_text(p, c, s, font, render::TextEffect::None, { 0,0,0,200 });
            },
            [](vector2 p, color_t c, std::string s, int font, int effect) {
                render::draw_text(p, c, s, font, render::TextEffect(effect), { 0,0,0,200 });
            },
            [](vector2 p, color_t c, std::string s, int font, int effect, color_t fx_c) {
                render::draw_text(p, c, s, font, render::TextEffect(effect), fx_c);
            }));

        r.set_function("get_text_size", [](sol::this_state ts, std::string s, sol::optional<int> font) {
            auto sz = render::get_text_size(s, font.value_or(render::kFont20));
            sol::state_view sv(ts);
            sol::table t = sv.create_table();
            t["w"] = sz.w; t["h"] = sz.h;
            return t;
        });
        r.set_function("get_char_advance", &render::get_char_advance);
        r.set_function("load_font_from_memory", &render::load_font_from_memory);
        r.set_function("load_font_from_file",   &render::load_font_from_file);

        r.set_function("create_bitmap",  &render::create_bitmap);
        r.set_function("destroy_bitmap", &render::destroy_bitmap);
        r.set_function("draw_bitmap",    sol::overload(
            [](render::bitmap_t id, vector2 a, vector2 b)                       { render::draw_bitmap(id, a, b, {255,255,255,255}, 0.f); },
            [](render::bitmap_t id, vector2 a, vector2 b, color_t tint)         { render::draw_bitmap(id, a, b, tint, 0.f); },
            [](render::bitmap_t id, vector2 a, vector2 b, color_t tint, float rnd){ render::draw_bitmap(id, a, b, tint, rnd); }));

        r.set_function("clip_push", &render::clip_push);
        r.set_function("clip_pop",  &render::clip_pop);
    }

    void bind_input() {
        auto in = lua_["input"].get_or_create<sol::table>();
        auto mouse = in["mouse"].get_or_create<sol::table>();
        mouse.set_function("get_position",         &input::mouse::get_position);
        mouse.set_function("get_position_desktop", &input::mouse::get_position_desktop);
        mouse.set_function("get_delta",            &input::mouse::get_delta);
        mouse.set_function("wheel_delta",          &input::mouse::wheel_delta);
        mouse.set_function("hover_rect",           &input::mouse::hover_rect);
        mouse.set_function("is_down",              &input::mouse::is_down);
        mouse.set_function("is_pressed",           &input::mouse::is_pressed);

        auto kb = in["keyboard"].get_or_create<sol::table>();
        kb.set_function("is_down",          &input::keyboard::is_down);
        kb.set_function("is_pressed",       &input::keyboard::is_pressed);
        kb.set_function("is_released",      &input::keyboard::is_released);
        kb.set_function("is_toggle_on",     &input::keyboard::is_toggle_on);
        kb.set_function("is_os_down",       &input::keyboard::is_os_down);
        kb.set_function("get_pressed_keys", &input::keyboard::get_pressed_keys);
        kb.set_function("vk_name",          &input::keyboard::vk_name);

        auto tx = in["text"].get_or_create<sol::table>();
        tx.set_function("get_recent", &input::text::get_recent);
    }

    void bind_fs() {
        auto fs = lua_["fs"].get_or_create<sol::table>();
        fs.set_function("create_file",      &fs::create_file);
        fs.set_function("create_directory", &fs::create_directory);
        fs.set_function("read_file",        [](const std::string& p) {
            auto r = fs::read_file(p);
            return std::make_pair(r.ok, r.data);
        });
        fs.set_function("does_file_exist", &fs::does_file_exist);
        fs.set_function("delete_file",     &fs::delete_file);
        fs.set_function("delete_directory",&fs::delete_directory);
        fs.set_function("query_directory", [](sol::this_state s, const std::string& p,
                                              sol::optional<bool> dirs, sol::optional<bool> files,
                                              sol::optional<sol::table> exts) {
            std::vector<std::string> exs;
            if (exts) for (auto& kv : *exts) exs.push_back(kv.second.as<std::string>());
            auto rows = fs::query_directory(p, dirs.value_or(true), files.value_or(true), exs);
            sol::state_view sv(s);
            sol::table out = sv.create_table();
            int i = 1;
            for (auto& e : rows) {
                sol::table row = sv.create_table();
                row["name"]         = e.name;
                row["is_directory"] = e.is_directory;
                row["size"]         = double(e.size);
                out[i++] = row;
            }
            return out;
        });
    }

    void bind_mathx() {
        auto m = lua_["mathx"].get_or_create<sol::table>();
        m["M_PI"]    = mathx::M_PI;
        m["M_TAU"]   = mathx::M_TAU;
        m["DEG2RAD"] = mathx::DEG2RAD;
        m["RAD2DEG"] = mathx::RAD2DEG;

        m.set_function("clamp",         [](float v, float mn, float mx){ return mathx::clamp(v, mn, mx); });
        m.set_function("saturate",      [](float v){ return mathx::saturate(v); });
        m.set_function("lerp",          [](float a, float b, float t){ return mathx::lerp(a, b, t); });
        m.set_function("smoothstep",    &mathx::smoothstep);
        m.set_function("remap",         &mathx::remap);
        m.set_function("wrap",          &mathx::wrap);
        m.set_function("inverse_lerp",  [](float a, float b, float v){ return mathx::inverse_lerp(a, b, v); });

        auto mat = lua_["mat4"].get_or_create<sol::table>();
        mat.set_function("identity",    &mathx::mat4::identity);
        mat.set_function("translation", &mathx::mat4::translation);
        mat.set_function("scaling",     &mathx::mat4::scaling);
        mat.set_function("rotation",    &mathx::mat4::rotation);
    }

    void bind_util() {
        auto u = lua_["util"].get_or_create<sol::table>();
        u.set_function("base64_encode", &util::base64_encode);
        u.set_function("base64_decode", [](sol::this_state ts, const std::string& s) -> sol::object {
            auto r = util::base64_decode(s);
            if (!r) return sol::lua_nil;
            return sol::make_object(ts, *r);
        });
        u.set_function("hex_encode", &util::hex_encode);
        u.set_function("hex_decode", [](sol::this_state ts, const std::string& s) -> sol::object {
            auto r = util::hex_decode(s);
            if (!r) return sol::lua_nil;
            return sol::make_object(ts, *r);
        });
        u.set_function("url_encode", &util::url_encode);
        u.set_function("url_decode", &util::url_decode);
        u.set_function("fnv1a64",    [](const std::string& s) { return double(util::fnv1a64(s)); });
    }

    // Recursive json ↔ lua bridge. Handles nested objects/arrays, primitives,
    // null → nil. Arrays are detected by "table has only sequential integer
    // keys starting at 1" — same rule perception.cx documents.
    static sol::object json_to_lua(sol::state_view sv, const json::value& v) {
        if (v.is_null())      return sol::lua_nil;
        if (v.is_boolean())   return sol::make_object(sv, v.get<bool>());
        if (v.is_number())    return sol::make_object(sv, v.get<double>());
        if (v.is_string())    return sol::make_object(sv, v.get<std::string>());
        if (v.is_array()) {
            sol::table t = sv.create_table();
            int i = 1;
            for (auto& item : v) t[i++] = json_to_lua(sv, item);
            return t;
        }
        // object
        sol::table t = sv.create_table();
        for (auto it = v.begin(); it != v.end(); ++it) {
            t[it.key()] = json_to_lua(sv, it.value());
        }
        return t;
    }

    static bool table_is_array(sol::table t) {
        // Sequential 1..N integer keys → JSON array. Anything else → object.
        int expected = 1;
        for (auto& kv : t) {
            auto k = kv.first;
            if (k.get_type() != sol::type::number) return false;
            double d = k.as<double>();
            if (d != double(int(d)) || int(d) != expected) return false;
            ++expected;
        }
        return expected > 1; // empty table encodes as {} object by default
    }

    static json::value lua_to_json(const sol::object& obj) {
        switch (obj.get_type()) {
            case sol::type::nil:      return nullptr;
            case sol::type::boolean:  return obj.as<bool>();
            case sol::type::number:   return obj.as<double>();
            case sol::type::string:   return obj.as<std::string>();
            case sol::type::table: {
                sol::table t = obj.as<sol::table>();
                if (table_is_array(t)) {
                    json::value arr = json::value::array();
                    for (auto& kv : t) arr.push_back(lua_to_json(kv.second));
                    return arr;
                }
                json::value o = json::value::object();
                for (auto& kv : t) {
                    std::string k;
                    if (kv.first.get_type() == sol::type::string)      k = kv.first.as<std::string>();
                    else if (kv.first.get_type() == sol::type::number) k = std::to_string(kv.first.as<double>());
                    else continue;
                    o[k] = lua_to_json(kv.second);
                }
                return o;
            }
            default: return nullptr;
        }
    }

    void bind_json() {
        auto j = lua_["json"].get_or_create<sol::table>();
        auto parse_impl = [](sol::this_state ts, const std::string& s) -> sol::object {
            auto p = json::parse(s);
            if (!p) return sol::lua_nil;
            return json_to_lua(sol::state_view(ts), *p);
        };
        auto stringify_impl = [](sol::object v) {
            return lua_to_json(v).dump();
        };
        j.set_function("parse",     parse_impl);
        j.set_function("decode",    parse_impl);
        j.set_function("stringify", stringify_impl);
        j.set_function("encode",    stringify_impl);
    }

    void bind_net() {
        auto n = lua_["net"].get_or_create<sol::table>();
        n.set_function("http_get", [](const std::string& url, sol::optional<int> t) {
            auto r = net::http_get(url, t.value_or(0));
            return std::make_tuple(r.ok, r.status, r.body);
        });
        n.set_function("http_post", [](const std::string& url, const std::string& ct,
                                       const std::string& body, sol::optional<int> t) {
            auto r = net::http_post(url, ct, body, t.value_or(0));
            return std::make_tuple(r.ok, r.status, r.body);
        });
        // Websocket support surfaces the low-level primitive; see net.hpp.
        // Skipping the sol usertype registration to keep this binding compact —
        // add it when your scripts actually need WS.
    }

    void bind_engine() {
        auto e = lua_["engine"].get_or_create<sol::table>();
        e.set_function("log",               &paris::log);
        e.set_function("log_error",         &paris::log_error);
        e.set_function("log_console",       &paris::log_console);
        e.set_function("log_console_error", &paris::log_console_error);
        e.set_function("get_user_name",     &paris::get_user_name);
    }

    void bind_callbacks() {
        auto c = lua_["callbacks"].get_or_create<sol::table>();
        c.set_function("add", [name = name_](const std::string& ev, sol::protected_function fn) {
            auto shared = std::make_shared<sol::protected_function>(std::move(fn));
            std::string scope = name;
            callbacks::add(ev, [shared, ev, scope]() {
                auto r = (*shared)();
                if (!r.valid()) {
                    sol::error err = r;
                    log_console_error("lua callback [" + scope + "/" + ev + "] " + err.what());
                }
            });
        });
    }

    // ---------- IDE module bindings ----------

    void bind_editor() {
        auto e = lua_["editor"].get_or_create<sol::table>();
        e.set_function("get_active_file",         &editor::get_active_file);
        e.set_function("get_active_file_content", &editor::get_active_file_content);
        e.set_function("get_active_language",     &editor::get_active_language);
        e.set_function("get_root_path",           &editor::get_root_path);
        e.set_function("get_selection_text",      &editor::get_selection_text);
        e.set_function("get_cursor_line",         &editor::get_cursor_line);
        e.set_function("get_cursor_col",          &editor::get_cursor_col);
        e.set_function("get_line_count",          &editor::get_line_count);
        e.set_function("get_line_text",           &editor::get_line_text);
        e.set_function("get_tab_count",           &editor::get_tab_count);
        e.set_function("get_tab_file",            &editor::get_tab_file);
        e.set_function("get_active_tab",          &editor::get_active_tab);
        e.set_function("get_open_files",          &editor::get_open_files);
        e.set_function("set_cursor_pos",          &editor::set_cursor_pos);
        e.set_function("set_selection",           &editor::set_selection);
        e.set_function("insert_text",             &editor::insert_text);
        e.set_function("replace_selection",       &editor::replace_selection);
        e.set_function("open_file",               &editor::open_file);
        e.set_function("save_active_file",        &editor::save_active_file);
        e.set_function("goto_line",               &editor::goto_line);
        e.set_function("show_notification",       &editor::show_notification);
        e.set_function("set_status",              &editor::set_status);
        e.set_function("send_chat_message",       &editor::send_chat_message);
    }

    void bind_clipboard() {
        auto c = lua_["clipboard"].get_or_create<sol::table>();
        c.set_function("get", &clipboard::get);
        c.set_function("set", &clipboard::set);
    }

    void bind_intellisense() {
        // Providers are auto-wired from `on_completion` / `on_hover` globals
        // during load. Nothing to expose imperatively here — kept as a stub
        // so scripts that call `intellisense.trigger(...)` in future versions
        // find the namespace.
        lua_["intellisense"].get_or_create<sol::table>();
    }

    void bind_ai() {
        auto a = lua_["ai"].get_or_create<sol::table>();
        a.set_function("get_active_model",       &ai::get_active_model);
        a.set_function("set_active_model",       &ai::set_active_model);
        a.set_function("get_chat_message_count", &ai::get_chat_message_count);
        a.set_function("get_chat_message",       [](sol::this_state ts, int i) {
            sol::state_view sv(ts);
            auto m = ai::get_chat_message(i);
            sol::table t = sv.create_table();
            switch (m.role) {
                case ai::Role::User:      t["role"] = "user"; break;
                case ai::Role::Assistant: t["role"] = "assistant"; break;
                case ai::Role::System:    t["role"] = "system"; break;
                case ai::Role::Tool:      t["role"] = "tool"; break;
            }
            t["content"] = m.content;
            return t;
        });
    }

    void bind_tools() {
        auto t = lua_["tools"].get_or_create<sol::table>();
        t.set_function("register",       [](const std::string& n, const std::string& d, sol::optional<std::string> js) {
            tools::register_tool(n, d, js.value_or(""));
        });
        t.set_function("register_param", [](const std::string& tool_, const std::string& n, const std::string& type,
                                           const std::string& d, sol::optional<bool> req) {
            tools::register_param(tool_, n, type, d, req.value_or(false));
        });
        t.set_function("unregister",     &tools::unregister);
    }

    void bind_settings() {
        auto s = lua_["settings"].get_or_create<sol::table>();
        s.set_function("get",        [](const std::string& k, sol::optional<std::string> d) {
            return settings::get(k, d.value_or(""));
        });
        s.set_function("set",        &settings::set);
        s.set_function("get_bool",   [](const std::string& k, sol::optional<bool> d) {
            return settings::get_bool(k, d.value_or(false));
        });
        s.set_function("set_bool",   &settings::set_bool);
        s.set_function("get_number", [](const std::string& k, sol::optional<double> d) {
            return settings::get_number(k, d.value_or(0.0));
        });
        s.set_function("set_number", &settings::set_number);
        s.set_function("get_color",  [](const std::string& k, sol::optional<sol::table> d) {
            color_t def{ 255, 255, 255, 255 };
            if (d) {
                sol::table td = *d;
                def = { td.get_or<int>(1, 255), td.get_or<int>(2, 255),
                        td.get_or<int>(3, 255), td.get_or<int>(4, 255) };
            }
            return settings::get_color(k, def);
        });
        s.set_function("set_color",  &settings::set_color);

        // Widget factories (only meaningful during on_settings_render).
        s.set_function("create_label",        &settings::create_label);
        s.set_function("create_separator",    &settings::create_separator);
        s.set_function("create_spacing",      &settings::create_spacing);
        s.set_function("create_checkbox",     &settings::create_checkbox);
        s.set_function("create_button",       [](const std::string& label, sol::protected_function fn) {
            auto shared = std::make_shared<sol::protected_function>(std::move(fn));
            settings::create_button(label, [shared]() {
                auto r = (*shared)();
                if (!r.valid()) {
                    sol::error err = r;
                    log_console_error(std::string("lua settings button ") + err.what());
                }
            });
        });
        s.set_function("create_slider",       &settings::create_slider);
        s.set_function("create_input_text",   &settings::create_input_text);
        s.set_function("create_text_area",    &settings::create_text_area);
        s.set_function("create_dropdown",     [](const std::string& l, const std::string& k, sol::table opts) {
            std::vector<std::string> v;
            for (auto& kv : opts) v.push_back(kv.second.as<std::string>());
            settings::create_dropdown(l, k, std::move(v));
        });
        s.set_function("create_color_picker", &settings::create_color_picker);
        s.set_function("create_keybind",      &settings::create_keybind);
        s.set_function("create_progress_bar", &settings::create_progress_bar);
    }

    void bind_terminal() {
        auto t = lua_["terminal"].get_or_create<sol::table>();
        t.set_function("create",       [](sol::optional<std::string> title) {
            return terminal::create(title.value_or(std::string{"terminal"}));
        });
        t.set_function("send_text",    &terminal::send_text);
        t.set_function("send_command", &terminal::send_command);
        t.set_function("read_output",  [](terminal::terminal_id_t id, sol::optional<int> n) {
            return terminal::read_output(id, n.value_or(4096));
        });
        t.set_function("clear",       &terminal::clear);
        t.set_function("close",       &terminal::close);
        t.set_function("set_active",  &terminal::set_active);
        t.set_function("get_active",  &terminal::get_active);
        t.set_function("count",       &terminal::count);
    }

    void redirect_print() {
        lua_.set_function("print", [n = name_](sol::variadic_args va) {
            std::string s = "[" + n + "] ";
            bool first = true;
            for (auto v : va) {
                if (!first) s += "\t"; first = false;
                auto os = v.get<sol::optional<std::string>>();
                if (os) { s += *os; continue; }
                auto od = v.get<sol::optional<double>>();
                if (od) { s += std::to_string(*od); continue; }
                auto ob = v.get<sol::optional<bool>>();
                if (ob) { s += (*ob ? "true" : "false"); continue; }
                s += "<value>";
            }
            paris::log_console(s);
        });
    }

    // ---------- IDE hook wiring ----------
    //
    // For each named hook the extension may define, register a typed callback
    // that unpacks the payload and calls it. All are auto-dropped when the
    // script unloads via callbacks::drop_section.
    void wire_ide_hooks() {
        wire_file_hook("on_file_opened",    "editor.file_opened");
        wire_file_hook("on_file_saved",     "editor.file_saved");
        wire_file_hook("on_tab_changed",    "editor.tab_changed");
        wire_buffer_hook("on_buffer_changed");

        // AI pipeline hooks — extensions declare them as free functions and
        // we adapt them to the C++ callback shape.
        wire_ai_hooks();
        wire_intellisense_hooks();
        wire_settings_render();
    }

    void wire_file_hook(const char* lua_name, const char* event) {
        sol::protected_function f = lua_[lua_name];
        if (!f.valid()) return;
        auto shared = std::make_shared<sol::protected_function>(std::move(f));
        std::string scope = name_;
        callbacks::add_any(event, [shared, scope, lua_name](const std::any& payload) {
            try {
                auto ev = std::any_cast<editor::FileEvent>(payload);
                auto r = (*shared)(ev.path);
                if (!r.valid()) {
                    sol::error err = r;
                    log_console_error(std::string("lua ") + lua_name + " [" + scope + "] " + err.what());
                }
            } catch (const std::bad_any_cast&) {}
        });
    }

    void wire_buffer_hook(const char* lua_name) {
        sol::protected_function f = lua_[lua_name];
        if (!f.valid()) return;
        auto shared = std::make_shared<sol::protected_function>(std::move(f));
        std::string scope = name_;
        callbacks::add_any("editor.buffer_changed", [shared, scope, lua_name](const std::any& payload) {
            try {
                auto ev = std::any_cast<editor::BufferEvent>(payload);
                auto r = (*shared)(ev.path, ev.line);
                if (!r.valid()) {
                    sol::error err = r;
                    log_console_error(std::string("lua ") + lua_name + " [" + scope + "] " + err.what());
                }
            } catch (const std::bad_any_cast&) {}
        });
    }

    void wire_ai_hooks() {
        std::string scope = name_;

        sol::protected_function before = lua_["on_ai_before_send"];
        if (before.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(before));
            ai::on_before_send([s, scope](std::string& prompt, std::string& sys) -> bool {
                auto r = (*s)(prompt, sys);
                if (!r.valid()) return true; // errors don't cancel — log and continue
                // Convention: return { ok, prompt?, system? } as a table, or a bool.
                if (r.get_type(0) == sol::type::boolean) return r.get<bool>(0);
                if (r.get_type(0) == sol::type::table) {
                    sol::table t = r.get<sol::table>(0);
                    if (t["prompt"].is<std::string>()) prompt = t.get<std::string>("prompt");
                    if (t["system"].is<std::string>()) sys    = t.get<std::string>("system");
                    return t.get_or<bool>("ok", true);
                }
                return true;
            });
        }

        sol::protected_function after = lua_["on_ai_after_response"];
        if (after.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(after));
            ai::on_after_response([s](std::string& response) {
                auto r = (*s)(response);
                if (r.valid() && r.get_type(0) == sol::type::string) response = r.get<std::string>(0);
            });
        }

        sol::protected_function tc = lua_["on_ai_tool_call"];
        if (tc.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(tc));
            ai::on_tool_call([s](const std::string& name, const std::string& args_json) -> std::string {
                auto r = (*s)(name, args_json);
                if (!r.valid()) return {};
                if (r.get_type(0) == sol::type::string) return r.get<std::string>(0);
                return {};
            });
        }

        sol::protected_function at = lua_["on_ai_after_tool"];
        if (at.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(at));
            ai::on_after_tool([s](const std::string& name, const std::string& args_json, const std::string& result) {
                (*s)(name, args_json, result);
            });
        }

        sol::protected_function inj = lua_["on_ai_system_inject"];
        if (inj.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(inj));
            ai::on_system_inject([s]() -> std::string {
                auto r = (*s)();
                if (r.valid() && r.get_type(0) == sol::type::string) return r.get<std::string>(0);
                return {};
            });
        }
    }

    void wire_intellisense_hooks() {
        sol::protected_function comp = lua_["on_completion"];
        if (comp.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(comp));
            intellisense::register_completion(
                [s](const std::string& file, const std::string& line, int col) -> std::vector<intellisense::Completion> {
                    std::vector<intellisense::Completion> out;
                    auto r = (*s)(file, line, col);
                    if (!r.valid() || r.get_type(0) != sol::type::table) return out;
                    sol::table t = r.get<sol::table>(0);
                    for (auto& kv : t) {
                        sol::table row = kv.second;
                        out.push_back({
                            row.get_or<std::string>("label",  ""),
                            row.get_or<std::string>("insert", ""),
                            row.get_or<std::string>("detail", "")
                        });
                    }
                    return out;
                });
        }

        sol::protected_function hov = lua_["on_hover"];
        if (hov.valid()) {
            auto s = std::make_shared<sol::protected_function>(std::move(hov));
            intellisense::register_hover(
                [s](const std::string& file, const std::string& word, int line) -> std::string {
                    auto r = (*s)(file, word, line);
                    if (r.valid() && r.get_type(0) == sol::type::string) return r.get<std::string>(0);
                    return {};
                });
        }
    }

    void wire_settings_render() {
        sol::protected_function render_fn = lua_["on_settings_render"];
        if (!render_fn.valid()) return;
        auto s = std::make_shared<sol::protected_function>(std::move(render_fn));
        std::string scope = name_;
        // The host calls `settings::begin_render(section)` then dispatches
        // `settings.render` — we invoke the script's callback, which fills
        // the widget buffer through create_* calls.
        callbacks::add_any("settings.render", [s, scope](const std::any& payload) {
            try {
                struct RenderPayload { double x, y, w; };
                auto ev = std::any_cast<RenderPayload>(payload);
                (*s)(ev.x, ev.y, ev.w);
            } catch (const std::bad_any_cast&) {}
        });
    }

    std::string             name_;
    sol::state              lua_;
    sol::protected_function on_frame_;
    sol::protected_function on_tick_;
    sol::protected_function on_unload_;
    sol::protected_function on_deactivate_;
};

std::unique_ptr<Script> make_lua_script(std::string name) {
    return std::make_unique<LuaScript>(std::move(name));
}

} // namespace paris
