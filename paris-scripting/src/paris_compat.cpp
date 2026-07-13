#include "paris/paris_compat.hpp"

#include "paris/callbacks.hpp"
#include "paris/game_profile.hpp"
#include "paris/render.hpp"
#include "paris/types.hpp"
#include "paris/ui.hpp"

#include <sol/sol.hpp>

#include <cmath>
#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace paris::paris_compat {

namespace {

// One-line no-op returning `nil`. Handy for the many undocumented APIs
// scripts poke at when a feature isn't wired up.
sol::object noop(sol::this_state) { return sol::lua_nil; }

// --- Utility: hex color parser (#RRGGBB / #RRGGBBAA) ---
color_t parse_hex_color(const std::string& s) {
    if (s.empty() || s[0] != '#') return {};
    auto hex = [](char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };
    color_t c;
    if (s.size() >= 7) {
        c.r = hex(s[1]) * 16 + hex(s[2]);
        c.g = hex(s[3]) * 16 + hex(s[4]);
        c.b = hex(s[5]) * 16 + hex(s[6]);
    }
    if (s.size() >= 9) {
        c.a = hex(s[7]) * 16 + hex(s[8]);
    }
    return c;
}

// Wraps a `paris::ui::Element` handle so scripts can chain
// `element:get_value():get()` / `:set_raw()` in the same shape as the leaked
// scripts expect. The intermediate `Value` object is generated on the fly.
struct ValueHandle {
    std::shared_ptr<ui::Element> element;
};

// A `Control` wraps an Element for the compat `gui.checkbox()` etc. entry
// points. It carries the same shared_ptr but exposes the alternate method
// names (`add_callback`, `get_value`, `add`, `reset`).
struct Control {
    std::shared_ptr<ui::Element>      element;
    std::vector<std::shared_ptr<Control>> children; // for add()
    std::string                       label;
};

// A path-based lookup context — mimics `gui.ctx:find("rage>anti-aim>...")`.
// The compat layer keeps a flat map of paths → controls; adds attach
// children to the parent's `children` vector.
struct GuiCtx {
    std::unordered_map<std::string, std::shared_ptr<Control>> controls;

    std::shared_ptr<Control> find(const std::string& path) {
        auto it = controls.find(path);
        if (it != controls.end()) return it->second;
        // Fabricate a placeholder so scripts calling `:add` on an unknown
        // path don't crash. It won't render, but the script keeps running.
        auto c = std::make_shared<Control>();
        controls[path] = c;
        return c;
    }
};

// One shared context per Lua state.
struct CompatState {
    std::shared_ptr<GuiCtx> ctx = std::make_shared<GuiCtx>();
};

// ---------- Named event dispatcher ----------
// `events.<name>:add(fn)` maps to callbacks::add("<name>", fn) internally.
// The event name is captured in the metatable so any name works.

struct EventDispatcher {
    std::string event_name;

    void add(sol::this_state ts, sol::protected_function fn) {
        auto shared = std::make_shared<sol::protected_function>(std::move(fn));
        std::string ev = event_name;
        callbacks::add(ev, [shared, ev]() {
            auto r = (*shared)();
            if (!r.valid()) {
                sol::error err = r;
                // Errors are logged via the engine module elsewhere; here we
                // silently ignore to avoid duplicating log messages.
                (void)err;
            }
        });
    }
};

// ---------- Entity wrappers ----------

struct EntityHandle {
    game::entity_id id = 0;

    game::EntityInfo info() const  { return game::inspect_entity(id); }
    game::WeaponInfo weapon() const{ return game::inspect_weapon(info().active_weapon); }

    bool     is_alive()   const { return info().alive; }
    bool     is_enemy()   const { return info().enemy; }
    bool     is_dormant() const { return info().dormant; }
    int      get_health() const { return info().health; }
    int      get_team()   const { return info().team; }
    std::string get_name() const { return info().name; }
    vector3  get_eye_pos()      const { return info().eye_pos; }
    vector3  get_abs_origin()   const { return info().origin; }
    vector3  get_abs_velocity() const { return info().velocity; }
    vector3  get_view_angles()  const { return info().view_angles; }
    EntityHandle get_active_weapon() const {
        EntityHandle w; w.id = info().active_weapon; return w;
    }
    int get_type()      const { return weapon().type; }
    int get_ammo()      const { return weapon().ammo; }
    std::string get_weapon_name() const { return weapon().name; }
};

struct NetChanWrap {
    bool  is_null()     const { return !game::get_netchan().valid; }
    float get_latency() const { return game::get_netchan().latency; }
};

struct UserCmdWrap {
    game::UserCmd* cmd = nullptr;

    vector3 get_viewangles() const { return cmd ? cmd->view_angles : vector3{}; }
    void    set_viewangles(vector3 v) { if (cmd) cmd->view_angles = v; }
    float   get_forwardmove() const { return cmd ? cmd->forward_move : 0.f; }
    float   get_leftmove()    const { return cmd ? cmd->side_move    : 0.f; }
    void    set_forwardmove(float f)  { if (cmd) cmd->forward_move = f; }
    void    set_leftmove(float f)     { if (cmd) cmd->side_move    = f; }
    void    rotate_movement(float yaw_delta) {
        if (!cmd) return;
        float rad = yaw_delta * 3.14159265358979f / 180.f;
        float c = std::cos(rad), s = std::sin(rad);
        float fm = cmd->forward_move, sm = cmd->side_move;
        cmd->forward_move = fm * c - sm * s;
        cmd->side_move    = fm * s + sm * c;
    }
};

} // namespace

void install(sol::state& lua) {
    // Idempotent guard.
    if (lua["_paris_compat_installed"].valid() && lua["_paris_compat_installed"].get<bool>()) return;
    lua["_paris_compat_installed"] = true;

    auto compat = std::make_shared<CompatState>();

    // ---------- Types exposed to scripts ----------

    lua.new_usertype<ValueHandle>("_ValueHandle",
        // element:get_value():get() returns another chainable that supports
        // :get_raw() / :set_raw() and direct usage.
        "get", [](ValueHandle& v) -> sol::object {
            // We just hand back the same handle — the caller chains with
            // :get_raw() (returns int), :set_raw() (mutates), or uses it as
            // a bool via the metatable.
            return sol::make_object(sol::this_state{}, v);
        },
        "set", [](ValueHandle& v, sol::object arg) {
            if (!v.element) return;
            using ui::ElementKind;
            switch (v.element->kind) {
                case ElementKind::Checkbox:    v.element->value = arg.as<bool>(); break;
                case ElementKind::SliderInt:   v.element->value = arg.as<int>(); break;
                case ElementKind::SliderDouble:v.element->value = arg.as<double>(); break;
                case ElementKind::Input:       v.element->value = arg.as<std::string>(); break;
                case ElementKind::ColorPicker: v.element->value = arg.as<color_t>(); break;
                default: break;
            }
        },
        "get_raw", [](ValueHandle& v) -> int {
            if (!v.element) return 0;
            using ui::ElementKind;
            switch (v.element->kind) {
                case ElementKind::Checkbox:    return std::get<bool>(v.element->value) ? 1 : 0;
                case ElementKind::SliderInt:   return std::get<int>(v.element->value);
                case ElementKind::SliderDouble:return int(std::get<double>(v.element->value));
                case ElementKind::SingleSelect:return std::get<int>(v.element->value);
                default: return 0;
            }
        },
        "set_raw", [](ValueHandle& v, int raw) {
            if (!v.element) return;
            using ui::ElementKind;
            switch (v.element->kind) {
                case ElementKind::Checkbox:    v.element->value = (raw != 0); break;
                case ElementKind::SliderInt:   v.element->value = raw; break;
                case ElementKind::SliderDouble:v.element->value = double(raw); break;
                case ElementKind::SingleSelect:v.element->value = raw; break;
                default: break;
            }
        });

    lua.new_usertype<Control>("_ParisControl",
        "get_value", [](Control& c) -> sol::object {
            return sol::make_object(sol::this_state{}, ValueHandle{ c.element });
        },
        "add", [](Control& c, std::shared_ptr<Control> child) {
            c.children.push_back(std::move(child));
        },
        "reset", [](Control&) {
            // No-op — scripts call this to "commit" attached children; our
            // renderer walks the panel tree freshly each frame.
        },
        "add_callback", [](Control&, sol::protected_function) {
            // Scripts hook value-change callbacks. Our elements don't fire
            // change events yet; treat as a stub. Extending is straightforward:
            // hold the function alongside the Element and fire on set().
        });

    lua.new_usertype<EntityHandle>("_ParisEntity",
        "is_alive",          &EntityHandle::is_alive,
        "is_enemy",          &EntityHandle::is_enemy,
        "is_dormant",        &EntityHandle::is_dormant,
        "get_health",        &EntityHandle::get_health,
        "get_team",          &EntityHandle::get_team,
        "get_name",          &EntityHandle::get_name,
        "get_eye_pos",       &EntityHandle::get_eye_pos,
        "get_abs_origin",    &EntityHandle::get_abs_origin,
        "get_abs_velocity",  &EntityHandle::get_abs_velocity,
        "get_view_angles",   &EntityHandle::get_view_angles,
        "get_active_weapon", &EntityHandle::get_active_weapon,
        "get_type",          &EntityHandle::get_type,
        "get_ammo",          &EntityHandle::get_ammo,
        "get_weapon_name",   &EntityHandle::get_weapon_name);

    lua.new_usertype<NetChanWrap>("_ParisNetChan",
        "is_null",     &NetChanWrap::is_null,
        "get_latency", &NetChanWrap::get_latency);

    lua.new_usertype<UserCmdWrap>("_ParisUserCmd",
        "get_viewangles",   &UserCmdWrap::get_viewangles,
        "set_viewangles",   &UserCmdWrap::set_viewangles,
        "get_forwardmove",  &UserCmdWrap::get_forwardmove,
        "get_leftmove",     &UserCmdWrap::get_leftmove,
        "set_forwardmove",  &UserCmdWrap::set_forwardmove,
        "set_leftmove",     &UserCmdWrap::set_leftmove,
        "rotate_movement",  &UserCmdWrap::rotate_movement);

    lua.new_usertype<EventDispatcher>("_ParisEvent",
        "add", &EventDispatcher::add);

    // ---------- gui.* ----------

    auto gui = lua["gui"].get_or_create<sol::table>();

    gui.set_function("control_id", [](const std::string& s) { return s; });

    gui.set_function("checkbox", [](const std::string& id) {
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::Checkbox;
        el->name = id;
        el->value = false;
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("slider", [](sol::variadic_args va) {
        // signature: slider(id, min, max, format_table?) OR slider(id, def, min, max)
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::SliderInt;
        el->name = va[0].get<std::string>();
        int def = 0, mn = 0, mx = 100;
        if (va.size() >= 3) { mn = va[1].get<int>(); mx = va[2].get<int>(); }
        if (va.size() >= 4 && va[3].get_type() == sol::type::number) def = va[3].get<int>();
        el->i_min = mn; el->i_max = mx; el->value = def;
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("color_picker", [](const std::string& id) {
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::ColorPicker;
        el->name = id;
        el->value = color_t{ 255, 255, 255, 255 };
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("button", [](const std::string& label, sol::protected_function fn) {
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::Button;
        el->name = label;
        auto shared = std::make_shared<sol::protected_function>(std::move(fn));
        el->on_click = [shared]() { (*shared)(); };
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("combo_box", [](const std::string& id, sol::table opts) {
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::SingleSelect;
        el->name = id;
        for (auto& kv : opts) el->options.push_back(kv.second.as<std::string>());
        el->value = 0;
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("text_input", [](const std::string& id) {
        auto ctrl = std::make_shared<Control>();
        auto el = std::make_shared<ui::Element>();
        el->kind = ui::ElementKind::Input;
        el->name = id;
        el->value = std::string{};
        ctrl->element = el;
        return ctrl;
    });

    gui.set_function("selectable", gui["combo_box"]);
    gui.set_function("label", [](const std::string& text) {
        auto ctrl = std::make_shared<Control>();
        ctrl->label = text;
        return ctrl;
    });

    // make_control / MakeControl / MakeControlEasy — wrap a raw element into
    // a "labeled" one. The label is just stored on the ctrl.
    auto make_control = [](const std::string& label, std::shared_ptr<Control> inner) {
        if (!inner) inner = std::make_shared<Control>();
        inner->label = label;
        return inner;
    };
    gui.set_function("make_control",    make_control);
    gui.set_function("MakeControl",     make_control);
    gui.set_function("MakeControlEasy", make_control);

    // notify / notification — surface via the engine log so paris' overlay
    // shows them.
    gui.set_function("notify", [](sol::variadic_args va) {
        std::string s;
        for (auto v : va) {
            auto o = v.get<sol::optional<std::string>>();
            if (o) s += *o;
        }
        if (!s.empty()) paris::log(s);
    });
    gui["notification"] = gui["notify"];

    // Aliases for the frequently-seen PascalCase variants.
    gui["Checkbox"] = gui["checkbox"];
    gui["Selectable"] = gui["combo_box"];

    // ctx:find(path)
    gui["ctx"] = sol::table(lua, sol::create);
    gui["ctx"]["find"] = [compat](sol::object /*self*/, const std::string& path) {
        return compat->ctx->find(path);
    };

    // ---------- events.* ----------
    // Any accessed field becomes a fresh dispatcher for that event name.
    auto events = lua.create_table();
    sol::table events_mt = lua.create_table();
    events_mt.set_function(sol::meta_function::index,
        [](sol::this_state ts, sol::table t, const std::string& key) -> sol::object {
            // Cache the dispatcher on the table so `events.foo:add(...)` is
            // consistent across accesses.
            sol::object cached = t.raw_get<sol::object>(key);
            if (cached.valid() && cached != sol::lua_nil) return cached;
            auto disp = std::make_shared<EventDispatcher>();
            disp->event_name = key;
            sol::object made = sol::make_object(ts, disp);
            t.raw_set(key, made);
            return made;
        });
    events[sol::metatable_key] = events_mt;
    lua["events"] = events;

    // ---------- draw.* ----------
    auto draw = lua["draw"].get_or_create<sol::table>();
    draw.set_function("color", [](sol::variadic_args va) -> color_t {
        if (va.size() == 1 && va[0].get_type() == sol::type::string) {
            return parse_hex_color(va[0].get<std::string>());
        }
        int r = va.size() > 0 ? va[0].get<int>() : 255;
        int g = va.size() > 1 ? va[1].get<int>() : 255;
        int b = va.size() > 2 ? va[2].get<int>() : 255;
        int a = va.size() > 3 ? va[3].get<int>() : 255;
        return { r, g, b, a };
    });
    draw["Color"] = draw["color"];

    draw.set_function("vec", [](float x, float y) { return vector2{ x, y }; });
    draw["Vec"] = draw["vec"];

    draw.set_function("rect", [](sol::variadic_args va) {
        // (x1, y1, x2, y2, color) OR (v1, v2, color)
        if (va.size() >= 5 && va[0].get_type() == sol::type::number) {
            vector2 a{ va[0].get<float>(), va[1].get<float>() };
            vector2 b{ va[2].get<float>(), va[3].get<float>() };
            render::draw_rect_filled(a, b, va[4].get<color_t>(), 0.f);
        } else if (va.size() >= 3) {
            render::draw_rect_filled(va[0].get<vector2>(), va[1].get<vector2>(),
                                     va[2].get<color_t>(), 0.f);
        }
    });
    draw["Rect"] = draw["rect"];

    // surface — nested table with `.g` graphics context; scripts read/write
    // `.g.anti_alias` and other flags. Provide a permissive table.
    sol::table surface = lua.create_table();
    surface["g"] = lua.create_table();
    draw["surface"] = surface;

    draw["textures"]    = lua.create_table();
    draw["fonts"]       = lua.create_table();
    draw["shaders"]     = lua.create_table();
    draw["font_gdi"]    = lua.create_table();
    draw["font_flags"]  = lua.create_table();
    draw["text_alignment"] = lua.create_table();
    draw["text_params"]    = lua.create_table();
    draw["svg_texture"]    = [](sol::this_state ts) { return sol::lua_nil; };
    draw["Texture"]        = [](sol::this_state ts, const std::string&) {
        // Stub bitmap — Create()/other methods no-op. Real support belongs
        // in the game_profile backend.
        sol::state_view sv(ts);
        sol::table t = sv.create_table();
        t.set_function("Create", []() {});
        return t;
    };
    draw.set_function("GetFrameTime",   []() { return 0.f; });
    draw.set_function("GetResourceDir", []() { return std::string{"."}; });
    draw.set_function("rounding",       []() {}); // usually a helper — no-op

    // ---------- game.* ----------
    auto game = lua["game"].get_or_create<sol::table>();
    sol::table engine_t = lua.create_table();
    engine_t.set_function("in_game", []() { return paris::game::is_in_game(); });
    engine_t.set_function("get_netchan", []() { return NetChanWrap{}; });
    game["engine"] = engine_t;

    sol::table gv = lua.create_table();
    gv.set_function("realtime",       []() { return paris::game::get_real_time(); });
    gv.set_function("curtime",        []() { return paris::game::get_curtime(); });
    gv.set_function("tickcount",      []() { return paris::game::get_tick(); });
    game["global_vars"] = gv;

    game["input"]                     = lua.create_table();
    game["physics_query_interface"]   = lua.create_table();

    // ---------- entities.* ----------
    auto ent = lua["entities"].get_or_create<sol::table>();
    ent.set_function("get_local_pawn", [](sol::this_state ts) {
        EntityHandle h; h.id = paris::game::get_local_pawn();
        return sol::make_object(ts, h);
    });
    ent.set_function("get_local_controller", [](sol::this_state ts) {
        EntityHandle h; h.id = paris::game::get_local_controller();
        return sol::make_object(ts, h);
    });

    // players/controllers with :for_each shape:
    sol::table players = lua.create_table();
    players.set_function("for_each", [](sol::this_state ts, sol::object /*self*/, sol::protected_function fn) {
        sol::state_view sv(ts);
        for (auto id : paris::game::get_players()) {
            sol::table row = sv.create_table();
            EntityHandle h; h.id = id;
            row["entity"] = h;
            fn(row);
        }
    });
    ent["players"] = players;

    sol::table controllers = lua.create_table();
    controllers.set_function("for_each", [](sol::this_state ts, sol::object /*self*/, sol::protected_function fn) {
        sol::state_view sv(ts);
        for (auto id : paris::game::get_controllers()) {
            sol::table row = sv.create_table();
            EntityHandle h; h.id = id;
            row["entity"] = h;
            fn(row);
        }
    });
    ent["controllers"] = controllers;

    // ---------- math.* extensions ----------
    auto math = lua["math"].get_or_create<sol::table>();
    math.set_function("vec3", [](float x, float y, float z) { return vector3{ x, y, z }; });
    math.set_function("vec2", [](float x, float y) { return vector2{ x, y }; });
    math.set_function("calc_angle", [](vector3 from, vector3 to) {
        vector3 d = to - from;
        float pitch = -std::atan2(d.z, std::sqrt(d.x * d.x + d.y * d.y)) * 180.f / 3.14159265f;
        float yaw   =  std::atan2(d.y, d.x) * 180.f / 3.14159265f;
        return vector3{ pitch, yaw, 0.f };
    });

    // ---------- utils / hvh / theme / mods / ant — stubs ----------
    // Metatables that return no-op function for any accessed key so scripts
    // referencing unknown fields don't blow up.
    auto permissive_ns = [&](const char* name) {
        sol::table t = lua.create_table();
        sol::table mt = lua.create_table();
        mt.set_function(sol::meta_function::index,
            [](sol::this_state ts, sol::table, const std::string&) -> sol::object {
                return sol::make_object(ts, [](sol::variadic_args) { return sol::lua_nil; });
            });
        t[sol::metatable_key] = mt;
        lua[name] = t;
    };
    permissive_ns("utils");
    permissive_ns("hvh");
    permissive_ns("theme");
    permissive_ns("mods");
    permissive_ns("ant");
    permissive_ns("play");
    permissive_ns("http");

    // ---------- ffi stub ----------
    // Standard Lua (not LuaJIT) so genuine ffi.cdef won't work. Provide a
    // no-op table that lets scripts probe with `if ffi and ffi.cdef` and
    // fall through gracefully.
    sol::table ffi = lua.create_table();
    ffi["cdef"]        = [](sol::variadic_args) {};
    ffi["cast"]        = [](sol::variadic_args) -> sol::object { return sol::lua_nil; };
    ffi["new"]         = [](sol::variadic_args) -> sol::object { return sol::lua_nil; };
    ffi["sizeof"]      = [](sol::variadic_args) -> int { return 0; };
    ffi["string"]      = [](sol::variadic_args) -> std::string { return ""; };
    ffi["metatype"]    = [](sol::variadic_args) -> sol::object { return sol::lua_nil; };
    ffi["typeof"]      = [](sol::variadic_args) -> sol::object { return sol::lua_nil; };
    ffi["load"]        = [](sol::variadic_args) -> sol::object { return sol::lua_nil; };
    lua["ffi"] = ffi;
    // Also route utils.find_export through the ffi stub for compat.
    lua["utils"]["find_export"] = [](sol::variadic_args) { return 0; };
    lua["utils"]["GetDate"]     = [](sol::this_state ts) {
        std::time_t t = std::time(nullptr);
        std::tm* lt = std::localtime(&t);
        sol::state_view sv(ts);
        sol::table row = sv.create_table();
        row["hour"]   = lt->tm_hour;
        row["minute"] = lt->tm_min;
        row["second"] = lt->tm_sec;
        row["year"]   = lt->tm_year + 1900;
        row["month"]  = lt->tm_mon + 1;
        row["day"]    = lt->tm_mday;
        return row;
    };
}

} // namespace paris::paris_compat
