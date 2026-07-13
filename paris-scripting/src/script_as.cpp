// AngelScript backend. Covers the core surface (types, ui basics, render, input
// basics, mathx, engine.log, callbacks). The Lua backend has the full API;
// scripts that need the rarer modules (fs, json, net, list/multi_select) are
// still better written in Lua for now. Extending AS is mechanical — add
// registrations following the patterns below.

#include "paris/ai.hpp"
#include "paris/callbacks.hpp"
#include "paris/clipboard.hpp"
#include "paris/editor.hpp"
#include "paris/engine.hpp"
#include "paris/fs.hpp"
#include "paris/input.hpp"
#include "paris/json.hpp"
#include "paris/mathx.hpp"
#include "paris/net.hpp"
#include "paris/render.hpp"
#include "paris/sandbox.hpp"
#include "paris/settings.hpp"
#include "paris/terminal.hpp"
#include "paris/tools.hpp"
#include "paris/types.hpp"
#include "paris/ui.hpp"
#include "paris/util.hpp"

#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptarray/scriptarray.h>

#include <atomic>
#include <memory>
#include <string>
#include <utility>

namespace paris {

// Forward decl — defined below.
asIScriptEngine* get_or_init_as_engine();

namespace {

void as_message_callback(const asSMessageInfo* msg, void*) {
    LogLevel lvl = LogLevel::Info;
    if      (msg->type == asMSGTYPE_WARNING) lvl = LogLevel::Warn;
    else if (msg->type == asMSGTYPE_ERROR)   lvl = LogLevel::Error;
    std::string txt = std::string(msg->section) + " ("
        + std::to_string(msg->row) + ", " + std::to_string(msg->col) + "): " + msg->message;
    if (lvl == LogLevel::Error) log_console_error(txt);
    else                        log_console(txt);
}

// Trampolines used with asFUNCTION for placement-new value type construction.
template <typename T, typename... Args>
static void ConstructAt(void* mem, Args... a) { new (mem) T(std::forward<Args>(a)...); }

// Ref-counted wrapper for UI handles.
template <typename Handle>
struct RC {
    std::atomic<int> ref{ 1 };
    Handle           inner;
    template <typename... A> RC(A&&... a) : inner(std::forward<A>(a)...) {}
    void AddRef()  { ref.fetch_add(1, std::memory_order_relaxed); }
    void Release() { if (ref.fetch_sub(1, std::memory_order_acq_rel) == 1) delete this; }
};

using RCCheckbox      = RC<ui::Checkbox>;
using RCSliderInt     = RC<ui::SliderInt>;
using RCSliderDouble  = RC<ui::SliderDouble>;
using RCButton        = RC<ui::Button>;
using RCColorPicker   = RC<ui::ColorPicker>;
using RCSubtab        = RC<std::shared_ptr<ui::Subtab>>;
using RCPanel         = RC<std::shared_ptr<ui::Panel>>;

// A callable AngelScript function bound as a callback.
struct ASCallable {
    asIScriptFunction* fn      = nullptr;
    asIScriptContext*  ctx     = nullptr;
    asIScriptEngine*   engine  = nullptr;
    std::string        section;
    std::string        event;

    ~ASCallable() {
        if (ctx) ctx->Release();
        if (fn)  fn->Release();
    }

    void invoke() {
        if (!fn) return;
        ctx->Prepare(fn);
        int r = ctx->Execute();
        if (r != asEXECUTION_FINISHED) {
            if (r == asEXECUTION_EXCEPTION)
                log_console_error("as [" + section + "/" + event + "] " + ctx->GetExceptionString());
            else
                log_console_error("as [" + section + "/" + event + "] execution failed");
        }
    }
};

// ---------- registrations ----------

void RegisterValueTypes(asIScriptEngine* e) {
    // vector2
    e->RegisterObjectType("vector2", sizeof(vector2),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<vector2>());
    e->RegisterObjectBehaviour("vector2", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION((ConstructAt<vector2>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("vector2", asBEHAVE_CONSTRUCT, "void f(float, float)",
        asFUNCTION((ConstructAt<vector2, float, float>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectProperty("vector2", "float x", asOFFSET(vector2, x));
    e->RegisterObjectProperty("vector2", "float y", asOFFSET(vector2, y));
    e->RegisterObjectMethod("vector2", "float length() const",
        asMETHOD(vector2, length), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "float length_sqr() const",
        asMETHOD(vector2, length_sqr), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "float distance(const vector2 &in) const",
        asMETHOD(vector2, distance), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "float dot(const vector2 &in) const",
        asMETHOD(vector2, dot), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "vector2 normalized() const",
        asMETHOD(vector2, normalized), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "vector2 opAdd(const vector2 &in) const",
        asMETHODPR(vector2, operator+, (const vector2&) const, vector2), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "vector2 opSub(const vector2 &in) const",
        asMETHODPR(vector2, operator-, (const vector2&) const, vector2), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "vector2 opMul(float) const",
        asMETHODPR(vector2, operator*, (float) const, vector2), asCALL_THISCALL);
    e->RegisterObjectMethod("vector2", "string to_string() const",
        asMETHOD(vector2, to_string), asCALL_THISCALL);
    e->RegisterTypedef("vec2_t", "vector2");

    // vector3
    e->RegisterObjectType("vector3", sizeof(vector3),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<vector3>());
    e->RegisterObjectBehaviour("vector3", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION((ConstructAt<vector3>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("vector3", asBEHAVE_CONSTRUCT, "void f(float, float, float)",
        asFUNCTION((ConstructAt<vector3, float, float, float>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectProperty("vector3", "float x", asOFFSET(vector3, x));
    e->RegisterObjectProperty("vector3", "float y", asOFFSET(vector3, y));
    e->RegisterObjectProperty("vector3", "float z", asOFFSET(vector3, z));
    e->RegisterObjectMethod("vector3", "float length() const",
        asMETHOD(vector3, length), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "float length_sqr() const",
        asMETHOD(vector3, length_sqr), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "float distance(const vector3 &in) const",
        asMETHOD(vector3, distance), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "float dot(const vector3 &in) const",
        asMETHOD(vector3, dot), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "vector3 cross(const vector3 &in) const",
        asMETHOD(vector3, cross), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "vector3 normalized() const",
        asMETHOD(vector3, normalized), asCALL_THISCALL);
    e->RegisterObjectMethod("vector3", "bool to_screen(vector2 &out) const",
        asMETHOD(vector3, to_screen), asCALL_THISCALL);
    e->RegisterTypedef("vec3_t", "vector3");

    // color_t
    e->RegisterObjectType("color_t", sizeof(color_t),
        asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<color_t>());
    e->RegisterObjectBehaviour("color_t", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION((ConstructAt<color_t>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("color_t", asBEHAVE_CONSTRUCT, "void f(int, int, int)",
        asFUNCTION((ConstructAt<color_t, int, int, int>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("color_t", asBEHAVE_CONSTRUCT, "void f(int, int, int, int)",
        asFUNCTION((ConstructAt<color_t, int, int, int, int>)), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectProperty("color_t", "int r", asOFFSET(color_t, r));
    e->RegisterObjectProperty("color_t", "int g", asOFFSET(color_t, g));
    e->RegisterObjectProperty("color_t", "int b", asOFFSET(color_t, b));
    e->RegisterObjectProperty("color_t", "int a", asOFFSET(color_t, a));
    e->RegisterObjectMethod("color_t", "color_t with_alpha(int) const",
        asMETHOD(color_t, with_alpha), asCALL_THISCALL);
    e->RegisterObjectMethod("color_t", "color_t lerp(const color_t &in, float) const",
        asMETHOD(color_t, lerp), asCALL_THISCALL);
    e->SetDefaultNamespace("color_t");
    e->RegisterGlobalFunction("color_t from_hsv(float, float, float)",
        asFUNCTION(color_t::from_hsv), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- UI ----------

template <typename RC>
void RegisterRef(asIScriptEngine* e, const char* n) {
    e->RegisterObjectType(n, 0, asOBJ_REF);
    e->RegisterObjectBehaviour(n, asBEHAVE_ADDREF,  "void f()", asMETHOD(RC, AddRef),  asCALL_THISCALL);
    e->RegisterObjectBehaviour(n, asBEHAVE_RELEASE, "void f()", asMETHOD(RC, Release), asCALL_THISCALL);
}

// Free functions the scripts see.
RCSubtab*   ui_create_subtab(int tab, const std::string& name)  { return new RCSubtab(ui::create_subtab(tab, name)); }
RCPanel*    subtab_add_panel(RCSubtab* s, const std::string& n, bool small) { return new RCPanel((*s->inner).add_panel(n, small)); }
void        subtab_set_active(RCSubtab* s, bool a)              { (*s->inner).set_active(a); }

RCCheckbox* panel_add_checkbox(RCPanel* p, const std::string& n, bool init) {
    return new RCCheckbox((*p->inner)->add_checkbox(n, init));
}
RCSliderInt* panel_add_slider_int(RCPanel* p, const std::string& n, const std::string& post,
                                  int v, int mn, int mx, int st) {
    return new RCSliderInt((*p->inner)->add_slider_int(n, post, v, mn, mx, st));
}
RCSliderDouble* panel_add_slider_double(RCPanel* p, const std::string& n, const std::string& post,
                                        double v, double mn, double mx, double st) {
    return new RCSliderDouble((*p->inner)->add_slider_double(n, post, v, mn, mx, st));
}
RCColorPicker* panel_add_color(RCPanel* p, RCCheckbox* parent, const std::string& n, color_t init) {
    return new RCColorPicker((*p->inner)->add_color(parent->inner, n, init));
}
RCButton* panel_add_button(RCPanel* p, const std::string& n, asIScriptFunction* fn) {
    auto callable    = std::make_shared<ASCallable>();
    callable->engine = get_or_init_as_engine();
    callable->fn     = fn;
    callable->ctx    = callable->engine->CreateContext();
    callable->section = callbacks::detail::current_section();
    callable->event   = "ui.button";
    return new RCButton((*p->inner)->add_button(n, [callable]() { callable->invoke(); }));
}

void RegisterUI(asIScriptEngine* e) {
    e->RegisterFuncdef("void callback_t()");

    RegisterRef<RCCheckbox>     (e, "Checkbox");
    RegisterRef<RCSliderInt>    (e, "SliderInt");
    RegisterRef<RCSliderDouble> (e, "SliderDouble");
    RegisterRef<RCButton>       (e, "Button");
    RegisterRef<RCColorPicker>  (e, "ColorPicker");
    RegisterRef<RCSubtab>       (e, "Subtab");
    RegisterRef<RCPanel>        (e, "Panel");

    e->RegisterObjectMethod("Checkbox", "bool get() const",
        asFUNCTION(+[](RCCheckbox* h){ return h->inner.get(); }), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Checkbox", "void set(bool)",
        asFUNCTION(+[](RCCheckbox* h, bool v){ h->inner.set(v); }), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Checkbox", "void set_active(bool)",
        asFUNCTION(+[](RCCheckbox* h, bool a){ h->inner.set_active(a); }), asCALL_CDECL_OBJFIRST);

    e->RegisterObjectMethod("SliderInt", "int get() const",
        asFUNCTION(+[](RCSliderInt* h){ return h->inner.get(); }), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("SliderInt", "void set(int)",
        asFUNCTION(+[](RCSliderInt* h, int v){ h->inner.set(v); }), asCALL_CDECL_OBJFIRST);

    e->RegisterObjectMethod("SliderDouble", "double get() const",
        asFUNCTION(+[](RCSliderDouble* h){ return h->inner.get(); }), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("SliderDouble", "void set(double)",
        asFUNCTION(+[](RCSliderDouble* h, double v){ h->inner.set(v); }), asCALL_CDECL_OBJFIRST);

    e->RegisterObjectMethod("ColorPicker", "color_t get() const",
        asFUNCTION(+[](RCColorPicker* h){ return h->inner.get(); }), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("ColorPicker", "void set(const color_t &in)",
        asFUNCTION(+[](RCColorPicker* h, const color_t& c){ h->inner.set(c); }), asCALL_CDECL_OBJFIRST);

    e->RegisterObjectMethod("Subtab", "Panel@ add_panel(const string &in, bool)",
        asFUNCTION(subtab_add_panel), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Subtab", "void set_active(bool)",
        asFUNCTION(subtab_set_active), asCALL_CDECL_OBJFIRST);

    e->RegisterObjectMethod("Panel", "Checkbox@ add_checkbox(const string &in, bool)",
        asFUNCTION(panel_add_checkbox), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Panel", "SliderInt@ add_slider_int(const string &in, const string &in, int, int, int, int)",
        asFUNCTION(panel_add_slider_int), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Panel", "SliderDouble@ add_slider_double(const string &in, const string &in, double, double, double, double)",
        asFUNCTION(panel_add_slider_double), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Panel", "ColorPicker@ add_color(Checkbox@, const string &in, color_t)",
        asFUNCTION(panel_add_color), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectMethod("Panel", "Button@ add_button(const string &in, callback_t @)",
        asFUNCTION(panel_add_button), asCALL_CDECL_OBJFIRST);

    e->SetDefaultNamespace("ui");
    e->RegisterGlobalFunction("Subtab@ create_subtab(int, const string &in)",
        asFUNCTION(ui_create_subtab), asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_active()", asFUNCTION(ui::is_active), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- render ----------

void RegisterRender(asIScriptEngine* e) {
    e->SetDefaultNamespace("render");
    e->RegisterGlobalFunction("vector2 get_view()",       asFUNCTION(render::get_view), asCALL_CDECL);
    e->RegisterGlobalFunction("float get_view_scale()",   asFUNCTION(render::get_view_scale), asCALL_CDECL);
    e->RegisterGlobalFunction("float get_fps()",          asFUNCTION(render::get_fps), asCALL_CDECL);

    e->RegisterGlobalFunction(
        "void draw_rect(const vector2 &in, const vector2 &in, const color_t &in, float t = 1.0f, float rnd = 0.0f)",
        asFUNCTIONPR(render::draw_rect, (const vector2&, const vector2&, const color_t&, float, float), void),
        asCALL_CDECL);
    e->RegisterGlobalFunction(
        "void draw_rect_filled(const vector2 &in, const vector2 &in, const color_t &in, float rnd = 0.0f)",
        asFUNCTIONPR(render::draw_rect_filled, (const vector2&, const vector2&, const color_t&, float), void),
        asCALL_CDECL);
    e->RegisterGlobalFunction(
        "void draw_line(const vector2 &in, const vector2 &in, const color_t &in, float t = 1.0f)",
        asFUNCTIONPR(render::draw_line, (const vector2&, const vector2&, const color_t&, float), void),
        asCALL_CDECL);
    e->RegisterGlobalFunction(
        "void draw_circle(const vector2 &in, float, const color_t &in, float t = 1.0f, int seg = 24)",
        asFUNCTIONPR(render::draw_circle, (const vector2&, float, const color_t&, float, int), void),
        asCALL_CDECL);
    e->RegisterGlobalFunction(
        "void draw_circle_filled(const vector2 &in, float, const color_t &in, int seg = 24)",
        asFUNCTIONPR(render::draw_circle_filled, (const vector2&, float, const color_t&, int), void),
        asCALL_CDECL);

    e->RegisterGlobalFunction(
        "void draw_text(const vector2 &in, const color_t &in, const string &in, int font = 20, int effect = 0)",
        asFUNCTION(+[](const vector2& p, const color_t& c, const std::string& s, int font, int effect) {
            render::draw_text(p, c, s, font, render::TextEffect(effect), { 0,0,0,200 });
        }), asCALL_CDECL);

    e->SetDefaultNamespace("");
}

// ---------- input ----------

void RegisterInput(asIScriptEngine* e) {
    e->SetDefaultNamespace("input");
    e->SetDefaultNamespace("input::mouse");
    e->RegisterGlobalFunction("vector2 get_position()", asFUNCTION(input::mouse::get_position), asCALL_CDECL);
    e->RegisterGlobalFunction("float wheel_delta()",   asFUNCTION(input::mouse::wheel_delta), asCALL_CDECL);
    e->RegisterGlobalFunction("bool hover_rect(const vector2 &in, const vector2 &in)",
        asFUNCTION(input::mouse::hover_rect), asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_down(int)",    asFUNCTION(input::mouse::is_down), asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_pressed(int)", asFUNCTION(input::mouse::is_pressed), asCALL_CDECL);

    e->SetDefaultNamespace("input::keyboard");
    e->RegisterGlobalFunction("bool is_down(int)",      asFUNCTION(input::keyboard::is_down),      asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_pressed(int)",   asFUNCTION(input::keyboard::is_pressed),   asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_released(int)",  asFUNCTION(input::keyboard::is_released),  asCALL_CDECL);
    e->RegisterGlobalFunction("bool is_toggle_on(int)", asFUNCTION(input::keyboard::is_toggle_on), asCALL_CDECL);

    e->SetDefaultNamespace("");
}

// ---------- engine (log / user) ----------

void RegisterEngineNs(asIScriptEngine* e) {
    e->SetDefaultNamespace("engine");
    e->RegisterGlobalFunction("void log(const string &in)",
        asFUNCTIONPR(paris::log, (const std::string&), void), asCALL_CDECL);
    e->RegisterGlobalFunction("void log_error(const string &in)",
        asFUNCTION(paris::log_error), asCALL_CDECL);
    e->RegisterGlobalFunction("void log_console(const string &in)",
        asFUNCTION(paris::log_console), asCALL_CDECL);
    e->RegisterGlobalFunction("void log_console_error(const string &in)",
        asFUNCTION(paris::log_console_error), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_user_name()",
        asFUNCTION(paris::get_user_name), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- callbacks ----------

void as_callbacks_add(const std::string& event, asIScriptFunction* fn) {
    auto callable    = std::make_shared<ASCallable>();
    callable->engine = get_or_init_as_engine();
    callable->fn     = fn;
    callable->ctx    = callable->engine->CreateContext();
    callable->section = callbacks::detail::current_section();
    callable->event   = event;
    callbacks::add(event, [callable]() { callable->invoke(); });
}

void RegisterCallbacksNs(asIScriptEngine* e) {
    e->SetDefaultNamespace("callbacks");
    e->RegisterGlobalFunction("void add(const string &in, callback_t @)",
        asFUNCTION(as_callbacks_add), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- mathx / util helpers ----------

void RegisterMathxNs(asIScriptEngine* e) {
    e->SetDefaultNamespace("mathx");
    e->RegisterGlobalProperty("const double M_PI",    (void*)&mathx::M_PI);
    e->RegisterGlobalProperty("const double M_TAU",   (void*)&mathx::M_TAU);
    e->RegisterGlobalProperty("const double DEG2RAD", (void*)&mathx::DEG2RAD);
    e->RegisterGlobalProperty("const double RAD2DEG", (void*)&mathx::RAD2DEG);

    e->RegisterGlobalFunction("float clamp(float, float, float)",
        asFUNCTION(+[](float v, float mn, float mx){ return mathx::clamp(v, mn, mx); }), asCALL_CDECL);
    e->RegisterGlobalFunction("float saturate(float)",
        asFUNCTION(+[](float v){ return mathx::saturate(v); }), asCALL_CDECL);
    e->RegisterGlobalFunction("float lerp(float, float, float)",
        asFUNCTION(+[](float a, float b, float t){ return mathx::lerp(a, b, t); }), asCALL_CDECL);
    e->RegisterGlobalFunction("float smoothstep(float, float, float)",
        asFUNCTION(mathx::smoothstep), asCALL_CDECL);
    e->RegisterGlobalFunction("float remap(float, float, float, float, float)",
        asFUNCTION(mathx::remap), asCALL_CDECL);
    e->RegisterGlobalFunction("float wrap(float, float, float)",
        asFUNCTION(mathx::wrap), asCALL_CDECL);
    e->RegisterGlobalFunction("float inverse_lerp(float, float, float)",
        asFUNCTION(+[](float a, float b, float v){ return mathx::inverse_lerp(a, b, v); }), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- fs / util / json / net (scaffolded) ----------

void RegisterFs(asIScriptEngine* e) {
    e->SetDefaultNamespace("fs");
    e->RegisterGlobalFunction("bool create_file(const string &in, const string &in)",
        asFUNCTION(fs::create_file), asCALL_CDECL);
    e->RegisterGlobalFunction("bool create_directory(const string &in)",
        asFUNCTION(fs::create_directory), asCALL_CDECL);
    e->RegisterGlobalFunction("bool does_file_exist(const string &in)",
        asFUNCTION(fs::does_file_exist), asCALL_CDECL);
    e->RegisterGlobalFunction("bool delete_file(const string &in)",
        asFUNCTION(fs::delete_file), asCALL_CDECL);
    e->RegisterGlobalFunction("bool delete_directory(const string &in)",
        asFUNCTION(fs::delete_directory), asCALL_CDECL);
    // read_file: signature deliberately returns a string; scripts test emptiness
    // to know if the read failed (matches perception's "ok, data" shape via
    // a single output for AS ergonomics).
    e->RegisterGlobalFunction("string read_file(const string &in)",
        asFUNCTION(+[](const std::string& p) {
            auto r = fs::read_file(p);
            return r.ok ? r.data : std::string{};
        }), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterUtil(asIScriptEngine* e) {
    e->SetDefaultNamespace("util");
    e->RegisterGlobalFunction("string base64_encode(const string &in)",
        asFUNCTION(util::base64_encode), asCALL_CDECL);
    e->RegisterGlobalFunction("string base64_decode(const string &in)",
        asFUNCTION(+[](const std::string& s) {
            auto r = util::base64_decode(s);
            return r ? *r : std::string{};
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string hex_encode(const string &in)",
        asFUNCTION(util::hex_encode), asCALL_CDECL);
    e->RegisterGlobalFunction("string hex_decode(const string &in)",
        asFUNCTION(+[](const std::string& s) {
            auto r = util::hex_decode(s);
            return r ? *r : std::string{};
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string url_encode(const string &in)",
        asFUNCTION(util::url_encode), asCALL_CDECL);
    e->RegisterGlobalFunction("string url_decode(const string &in)",
        asFUNCTION(util::url_decode), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterJson(asIScriptEngine* e) {
    // AS surface: only string-in/string-out validation. Deep table access
    // needs a native JSON reflection layer that AS doesn't have out of the
    // box — scripts pass strings around and use their own parser for typed
    // access. For rich JSON work, prefer Lua.
    e->SetDefaultNamespace("json");
    e->RegisterGlobalFunction("string parse(const string &in)",
        asFUNCTION(+[](const std::string& s) {
            auto p = json::parse(s);
            return p ? p->dump() : std::string{};
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string decode(const string &in)",
        asFUNCTION(+[](const std::string& s) {
            auto p = json::parse(s);
            return p ? p->dump() : std::string{};
        }), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

// ---------- IDE modules (editor / clipboard / ai / tools / settings / terminal) ----------

void RegisterEditor(asIScriptEngine* e) {
    e->SetDefaultNamespace("editor");
    e->RegisterGlobalFunction("string get_active_file()",         asFUNCTION(editor::get_active_file), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_active_file_content()", asFUNCTION(editor::get_active_file_content), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_active_language()",     asFUNCTION(editor::get_active_language), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_root_path()",           asFUNCTION(editor::get_root_path), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_selection_text()",      asFUNCTION(editor::get_selection_text), asCALL_CDECL);
    e->RegisterGlobalFunction("int    get_cursor_line()",         asFUNCTION(editor::get_cursor_line), asCALL_CDECL);
    e->RegisterGlobalFunction("int    get_cursor_col()",          asFUNCTION(editor::get_cursor_col), asCALL_CDECL);
    e->RegisterGlobalFunction("int    get_line_count()",          asFUNCTION(editor::get_line_count), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_line_text(int)",        asFUNCTION(editor::get_line_text), asCALL_CDECL);
    e->RegisterGlobalFunction("int    get_tab_count()",           asFUNCTION(editor::get_tab_count), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_tab_file(int)",         asFUNCTION(editor::get_tab_file), asCALL_CDECL);
    e->RegisterGlobalFunction("int    get_active_tab()",          asFUNCTION(editor::get_active_tab), asCALL_CDECL);
    e->RegisterGlobalFunction("void   set_cursor_pos(int, int)",  asFUNCTION(editor::set_cursor_pos), asCALL_CDECL);
    e->RegisterGlobalFunction("void   set_selection(int, int, int, int)", asFUNCTION(editor::set_selection), asCALL_CDECL);
    e->RegisterGlobalFunction("void   insert_text(const string &in)",       asFUNCTION(editor::insert_text), asCALL_CDECL);
    e->RegisterGlobalFunction("void   replace_selection(const string &in)", asFUNCTION(editor::replace_selection), asCALL_CDECL);
    e->RegisterGlobalFunction("void   open_file(const string &in)",         asFUNCTION(editor::open_file), asCALL_CDECL);
    e->RegisterGlobalFunction("void   save_active_file()",                  asFUNCTION(editor::save_active_file), asCALL_CDECL);
    e->RegisterGlobalFunction("void   goto_line(int)",                      asFUNCTION(editor::goto_line), asCALL_CDECL);
    e->RegisterGlobalFunction("void   show_notification(const string &in)", asFUNCTION(editor::show_notification), asCALL_CDECL);
    e->RegisterGlobalFunction("void   set_status(const string &in)",        asFUNCTION(editor::set_status), asCALL_CDECL);
    e->RegisterGlobalFunction("void   send_chat_message(const string &in)", asFUNCTION(editor::send_chat_message), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterClipboard(asIScriptEngine* e) {
    e->SetDefaultNamespace("clipboard");
    e->RegisterGlobalFunction("string get()",                  asFUNCTION(clipboard::get), asCALL_CDECL);
    e->RegisterGlobalFunction("void   set(const string &in)",  asFUNCTION(clipboard::set), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterAi(asIScriptEngine* e) {
    e->SetDefaultNamespace("ai");
    e->RegisterGlobalFunction("string get_active_model()",    asFUNCTION(ai::get_active_model), asCALL_CDECL);
    e->RegisterGlobalFunction("void set_active_model(const string &in)",
        asFUNCTION(ai::set_active_model), asCALL_CDECL);
    e->RegisterGlobalFunction("int get_chat_message_count()", asFUNCTION(ai::get_chat_message_count), asCALL_CDECL);
    // Message accessor returns role + content as separate calls to avoid
    // struct registration overhead.
    e->RegisterGlobalFunction("string get_chat_message_role(int)",
        asFUNCTION(+[](int i){
            auto m = ai::get_chat_message(i);
            switch (m.role) {
                case ai::Role::User:      return std::string("user");
                case ai::Role::Assistant: return std::string("assistant");
                case ai::Role::System:    return std::string("system");
                case ai::Role::Tool:      return std::string("tool");
            }
            return std::string{};
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string get_chat_message_content(int)",
        asFUNCTION(+[](int i){ return ai::get_chat_message(i).content; }), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterTools(asIScriptEngine* e) {
    e->SetDefaultNamespace("tools");
    e->RegisterGlobalFunction(
        "void register_tool(const string &in, const string &in, const string &in = \"\")",
        asFUNCTION(tools::register_tool), asCALL_CDECL);
    e->RegisterGlobalFunction(
        "void register_param(const string &in, const string &in, const string &in, const string &in, bool required = false)",
        asFUNCTION(tools::register_param), asCALL_CDECL);
    e->RegisterGlobalFunction("void unregister(const string &in)",
        asFUNCTION(tools::unregister), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterSettings(asIScriptEngine* e) {
    e->SetDefaultNamespace("settings");
    e->RegisterGlobalFunction("string get(const string &in, const string &in = \"\")",
        asFUNCTION(settings::get), asCALL_CDECL);
    e->RegisterGlobalFunction("void set(const string &in, const string &in)",
        asFUNCTION(settings::set), asCALL_CDECL);
    e->RegisterGlobalFunction("bool get_bool(const string &in, bool = false)",
        asFUNCTION(settings::get_bool), asCALL_CDECL);
    e->RegisterGlobalFunction("void set_bool(const string &in, bool)",
        asFUNCTION(settings::set_bool), asCALL_CDECL);
    e->RegisterGlobalFunction("double get_number(const string &in, double = 0.0)",
        asFUNCTION(settings::get_number), asCALL_CDECL);
    e->RegisterGlobalFunction("void set_number(const string &in, double)",
        asFUNCTION(settings::set_number), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_label(const string &in)",
        asFUNCTION(settings::create_label), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_separator()",
        asFUNCTION(settings::create_separator), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_spacing(double)",
        asFUNCTION(settings::create_spacing), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_checkbox(const string &in, const string &in)",
        asFUNCTION(settings::create_checkbox), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_slider(const string &in, const string &in, double, double, double)",
        asFUNCTION(settings::create_slider), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_input_text(const string &in, const string &in)",
        asFUNCTION(settings::create_input_text), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_text_area(const string &in, const string &in, int)",
        asFUNCTION(settings::create_text_area), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_color_picker(const string &in, const string &in)",
        asFUNCTION(settings::create_color_picker), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_keybind(const string &in, const string &in)",
        asFUNCTION(settings::create_keybind), asCALL_CDECL);
    e->RegisterGlobalFunction("void create_progress_bar(const string &in, double, double)",
        asFUNCTION(settings::create_progress_bar), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterTerminal(asIScriptEngine* e) {
    e->SetDefaultNamespace("terminal");
    e->RegisterGlobalFunction("int create(const string &in = \"terminal\")",
        asFUNCTION(terminal::create), asCALL_CDECL);
    e->RegisterGlobalFunction("void send_text(int, const string &in)",
        asFUNCTION(terminal::send_text), asCALL_CDECL);
    e->RegisterGlobalFunction("void send_command(int, const string &in)",
        asFUNCTION(terminal::send_command), asCALL_CDECL);
    e->RegisterGlobalFunction("string read_output(int, int = 4096)",
        asFUNCTION(terminal::read_output), asCALL_CDECL);
    e->RegisterGlobalFunction("void clear(int)",       asFUNCTION(terminal::clear), asCALL_CDECL);
    e->RegisterGlobalFunction("void close(int)",       asFUNCTION(terminal::close), asCALL_CDECL);
    e->RegisterGlobalFunction("void set_active(int)",  asFUNCTION(terminal::set_active), asCALL_CDECL);
    e->RegisterGlobalFunction("int  get_active()",     asFUNCTION(terminal::get_active), asCALL_CDECL);
    e->RegisterGlobalFunction("int  count()",          asFUNCTION(terminal::count), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

void RegisterNet(asIScriptEngine* e) {
    e->SetDefaultNamespace("net");
    e->RegisterGlobalFunction("int http_get_status(const string &in, int t = 15000)",
        asFUNCTION(+[](const std::string& url, int t) {
            auto r = net::http_get(url, t);
            return r.status;
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string http_get_body(const string &in, int t = 15000)",
        asFUNCTION(+[](const std::string& url, int t) {
            auto r = net::http_get(url, t);
            return r.ok ? r.body : std::string{};
        }), asCALL_CDECL);
    e->RegisterGlobalFunction("string http_post_body(const string &in, const string &in, const string &in, int t = 15000)",
        asFUNCTION(+[](const std::string& url, const std::string& ct, const std::string& body, int t) {
            auto r = net::http_post(url, ct, body, t);
            return r.ok ? r.body : std::string{};
        }), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

} // anonymous namespace

// ---------- AS script class ----------

class ASScript final : public Script {
public:
    ASScript(std::string name, asIScriptEngine* engine)
        : name_(std::move(name)), engine_(engine)
    {
        module_ = engine_->GetModule(name_.c_str(), asGM_ALWAYS_CREATE);
    }

    ~ASScript() override {
        if (ctx_) ctx_->Release();
        if (engine_ && module_) engine_->DiscardModule(name_.c_str());
    }

    bool load(const std::string& source) override {
        callbacks::detail::push_section(name_);
        ui::detail::push_section(name_);

        int r = module_->AddScriptSection(name_.c_str(), source.c_str(), source.size());
        if (r < 0) { log_console_error("as add section [" + name_ + "] failed"); pop(); return false; }
        r = module_->Build();
        if (r < 0) { log_console_error("as build [" + name_ + "] failed"); pop(); return false; }

        // Two entry point conventions: `main()` for overlay scripts,
        // `on_activate()` for IDE extensions. Whichever the script defines runs.
        asIScriptFunction* activate_fn   = module_->GetFunctionByDecl("void on_activate()");
        asIScriptFunction* main_fn       = module_->GetFunctionByDecl("int main()");
        asIScriptFunction* void_main_fn  = module_->GetFunctionByDecl("void main()");
        asIScriptFunction* run_fn        = activate_fn ? activate_fn : (main_fn ? main_fn : void_main_fn);

        int rc = 1;
        if (run_fn) {
            ctx_ = engine_->CreateContext();
            ctx_->Prepare(run_fn);
            int er = ctx_->Execute();
            if (er != asEXECUTION_FINISHED) {
                if (er == asEXECUTION_EXCEPTION)
                    log_console_error("as main/on_activate [" + name_ + "] " + ctx_->GetExceptionString());
                else
                    log_console_error("as main/on_activate [" + name_ + "] failed");
                pop(); return false;
            }
            if (main_fn) rc = ctx_->GetReturnDWord();
        }

        on_frame_  = module_->GetFunctionByDecl("int on_frame()");
        if (!on_frame_) on_frame_ = module_->GetFunctionByDecl("void on_frame()");
        on_tick_   = module_->GetFunctionByDecl("void on_tick()");
        on_unload_     = module_->GetFunctionByDecl("void on_unload()");
        on_deactivate_ = module_->GetFunctionByDecl("void on_deactivate()");

        wire_ide_hooks();

        pop();
        // Persist when any of the frame-loop hooks exist, or when the script
        // came in via on_activate (event-driven extension).
        return (activate_fn != nullptr) || (on_frame_ != nullptr) || (on_tick_ != nullptr) || rc > 0;
    }

    static void as_timeout_line_cb(asIScriptContext* ctx, void*) {
        if (sandbox::over_budget()) ctx->Abort();
    }

    bool tick() override {
        asIScriptFunction* fn = on_tick_ ? on_tick_ : on_frame_;
        if (!fn) return true;
        if (!ctx_) ctx_ = engine_->CreateContext();
        ctx_->SetLineCallback(asFUNCTION(as_timeout_line_cb), nullptr, asCALL_CDECL);
        callbacks::detail::push_section(name_);
        sandbox::begin_hook();
        ctx_->Prepare(fn);
        int er = ctx_->Execute();
        callbacks::detail::pop_section();
        if (er != asEXECUTION_FINISHED) {
            if (er == asEXECUTION_EXCEPTION)
                log_console_error("as tick [" + name_ + "] " + ctx_->GetExceptionString());
            return false;
        }
        if (fn->GetReturnTypeId() == asTYPEID_INT32)
            return int(ctx_->GetReturnDWord()) > 0;
        return true;
    }

    void unload_notify() override {
        auto run = [&](asIScriptFunction* fn, const char* label) {
            if (!fn) return;
            if (!ctx_) ctx_ = engine_->CreateContext();
            callbacks::detail::push_section(name_);
            ctx_->Prepare(fn);
            ctx_->Execute();
            callbacks::detail::pop_section();
            (void)label;
        };
        run(on_deactivate_, "on_deactivate");
        run(on_unload_,     "on_unload");

        // Drop IDE-scoped registrations owned by this script.
        intellisense::drop_section(name_);
        ai::drop_section(name_);
        tools::drop_section(name_);
        settings::drop_section(name_);
    }

    const std::string& name()     const override { return name_; }
    const std::string& language() const override { static const std::string s = "angelscript"; return s; }

private:
    void pop() {
        callbacks::detail::pop_section();
        ui::detail::pop_section();
    }

    // Wire event hooks by looking up specific AS declarations and registering
    // a callbacks::add_any handler that calls them via a private context.
    void wire_ide_hooks() {
        wire_file("void on_file_opened(const string &in)",    "editor.file_opened");
        wire_file("void on_file_saved(const string &in)",     "editor.file_saved");
        wire_file("void on_tab_changed(const string &in)",    "editor.tab_changed");
        wire_buffer("void on_buffer_changed(const string &in, int)");
    }

    void wire_file(const char* decl, const char* event) {
        asIScriptFunction* fn = module_->GetFunctionByDecl(decl);
        if (!fn) return;
        auto* eng = engine_;
        std::string scope = name_;
        callbacks::add_any(event, [eng, fn, scope, event](const std::any& payload) {
            try {
                auto ev = std::any_cast<editor::FileEvent>(payload);
                auto* ctx = eng->CreateContext();
                ctx->Prepare(fn);
                ctx->SetArgObject(0, &const_cast<std::string&>(ev.path));
                if (ctx->Execute() == asEXECUTION_EXCEPTION)
                    log_console_error(std::string("as [") + scope + "/" + event + "] " + ctx->GetExceptionString());
                ctx->Release();
            } catch (const std::bad_any_cast&) {}
        });
    }

    void wire_buffer(const char* decl) {
        asIScriptFunction* fn = module_->GetFunctionByDecl(decl);
        if (!fn) return;
        auto* eng = engine_;
        std::string scope = name_;
        callbacks::add_any("editor.buffer_changed", [eng, fn, scope](const std::any& payload) {
            try {
                auto ev = std::any_cast<editor::BufferEvent>(payload);
                auto* ctx = eng->CreateContext();
                ctx->Prepare(fn);
                ctx->SetArgObject(0, &const_cast<std::string&>(ev.path));
                ctx->SetArgDWord(1, ev.line);
                if (ctx->Execute() == asEXECUTION_EXCEPTION)
                    log_console_error(std::string("as [") + scope + "/buffer_changed] " + ctx->GetExceptionString());
                ctx->Release();
            } catch (const std::bad_any_cast&) {}
        });
    }

    std::string        name_;
    asIScriptEngine*   engine_ = nullptr;
    asIScriptModule*   module_ = nullptr;
    asIScriptContext*  ctx_    = nullptr;
    asIScriptFunction* on_frame_      = nullptr;
    asIScriptFunction* on_tick_       = nullptr;
    asIScriptFunction* on_unload_     = nullptr;
    asIScriptFunction* on_deactivate_ = nullptr;
};

// ---------- shared engine + factory ----------

namespace { asIScriptEngine* g_as_engine = nullptr; }

asIScriptEngine* get_or_init_as_engine() {
    if (g_as_engine) return g_as_engine;
    g_as_engine = asCreateScriptEngine();
    g_as_engine->SetMessageCallback(asFUNCTION(as_message_callback), nullptr, asCALL_CDECL);
    RegisterStdString(g_as_engine);
    RegisterScriptArray(g_as_engine, true);
    RegisterValueTypes(g_as_engine);
    RegisterUI(g_as_engine);
    RegisterRender(g_as_engine);
    RegisterInput(g_as_engine);
    RegisterEngineNs(g_as_engine);
    RegisterCallbacksNs(g_as_engine);
    RegisterMathxNs(g_as_engine);
    RegisterFs(g_as_engine);
    RegisterUtil(g_as_engine);
    RegisterJson(g_as_engine);
    RegisterNet(g_as_engine);
    RegisterEditor(g_as_engine);
    RegisterClipboard(g_as_engine);
    RegisterAi(g_as_engine);
    RegisterTools(g_as_engine);
    RegisterSettings(g_as_engine);
    RegisterTerminal(g_as_engine);
    return g_as_engine;
}

std::unique_ptr<Script> make_as_script(std::string name) {
    return std::make_unique<ASScript>(std::move(name), get_or_init_as_engine());
}

void shutdown_as_engine() {
    if (g_as_engine) {
        g_as_engine->ShutDownAndRelease();
        g_as_engine = nullptr;
    }
}

} // namespace paris
