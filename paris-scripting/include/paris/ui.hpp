#pragma once

#include "types.hpp"

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

// The `ui` module mirrors perception.cx's shape: 5 top-level tabs indexed 0–4,
// each holding a stack of subtabs; each subtab hosts panels; each panel hosts
// interactive elements. Scripts create elements once at load and read them from
// on_frame.

namespace paris::ui {

enum class ElementKind : int {
    Checkbox     = 0,
    SliderInt    = 1,
    SliderDouble = 2,
    Input        = 3,
    MultiSelect  = 4,
    SingleSelect = 5,
    Keybind      = 6,
    ColorPicker  = 7,
    Button       = 8,
    List         = 9,
};

enum class KeybindMode : int {
    Off       = 0,
    On        = 1,   // fires while held
    Single    = 2,   // fires once per press
    Toggle    = 3,   // toggled by press
    AlwaysOn  = 4,
};

struct ListRow {
    std::string name;
    std::string info;
    bool        highlighted = false;
};

struct Element {
    ElementKind kind;
    std::string name;
    std::string section; // owning script

    bool        active     = true; // visibility (set_active)
    bool        draw_title = true;
    bool        find_protect = false;
    bool        draw_just_label = false;

    // Universal value slot. The variant kind matches the ElementKind.
    std::variant<
        bool,                            // Checkbox
        int,                             // SliderInt / SingleSelect / Keybind vk (via .value)
        double,                          // SliderDouble
        std::string,                     // Input
        std::vector<bool>,               // MultiSelect
        color_t,                         // ColorPicker
        std::vector<ListRow>             // List
    > value{};

    // Slider bounds.
    double d_min = 0.0, d_max = 0.0, d_step = 1.0;
    int    i_min = 0,   i_max = 0,   i_step = 1;
    std::string postfix;

    // Options for select / list.
    std::vector<std::string> options;

    // Keybind state.
    int         keybind_vk   = 0;
    KeybindMode keybind_mode = KeybindMode::Off;
    bool        keybind_state = false; // toggle latch — the menu updates this

    // Button click callback. Ignored on other kinds.
    std::function<void()> on_click;

    // Parent element for children (color/keybind attached to a checkbox).
    Element* parent = nullptr;
    std::vector<std::shared_ptr<Element>> children;
};

// ---------- Handles (what scripts hold) ----------

class Checkbox {
public:
    explicit Checkbox(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    std::shared_ptr<Element> raw() const { return e_; }
    bool get() const           { return std::get<bool>(e_->value); }
    void set(bool v)           { e_->value = v; }
    void set_active(bool a)    { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class SliderInt {
public:
    explicit SliderInt(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    int  get() const           { return std::get<int>(e_->value); }
    void set(int v)            { e_->value = v; }
    void set_active(bool a)    { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class SliderDouble {
public:
    explicit SliderDouble(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    double get() const         { return std::get<double>(e_->value); }
    void   set(double v)       { e_->value = v; }
    void   set_active(bool a)  { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class Input {
public:
    explicit Input(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    std::string get() const    { return std::get<std::string>(e_->value); }
    void        set(std::string s) { e_->value = std::move(s); }
    void        set_active(bool a) { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class MultiSelect {
public:
    explicit MultiSelect(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    std::vector<bool> get() const   { return std::get<std::vector<bool>>(e_->value); }
    void              set(int idx, bool v) {
        auto& vec = std::get<std::vector<bool>>(e_->value);
        if (idx >= 0 && idx < int(vec.size())) vec[idx] = v;
    }
    void set_active(bool a) { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class SingleSelect {
public:
    explicit SingleSelect(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    int  get() const           { return std::get<int>(e_->value); }
    void set(int idx)          { e_->value = idx; }
    void set_active(bool a)    { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class Keybind {
public:
    explicit Keybind(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    bool is_pressed() const    { return e_->keybind_state; }
    // {vk, mode} pair.
    struct View { int vk; KeybindMode mode; };
    View get() const           { return { e_->keybind_vk, e_->keybind_mode }; }
    void set(int vk, KeybindMode m) { e_->keybind_vk = vk; e_->keybind_mode = m; }
    void set_active(bool a)    { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class ColorPicker {
public:
    explicit ColorPicker(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    color_t get() const        { return std::get<color_t>(e_->value); }
    void    set(color_t c)     { e_->value = c; }
    void    set_active(bool a) { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class Button {
public:
    explicit Button(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    void set_active(bool a)    { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
};

class List {
public:
    explicit List(std::shared_ptr<Element> e) : e_(std::move(e)) {}
    void append(std::string name, std::string info) {
        auto& rows = std::get<std::vector<ListRow>>(e_->value);
        rows.push_back({ std::move(name), std::move(info), false });
    }
    void append_after(std::string name, std::string info, int idx) {
        auto& rows = std::get<std::vector<ListRow>>(e_->value);
        if (idx < 0) idx = 0;
        if (idx > int(rows.size())) idx = int(rows.size());
        rows.insert(rows.begin() + idx, { std::move(name), std::move(info), false });
    }
    int get() const { return active_idx_; }
    std::vector<ListRow> get_all() const { return std::get<std::vector<ListRow>>(e_->value); }
    int  get_count() const  { return int(std::get<std::vector<ListRow>>(e_->value).size()); }
    void clear()            { std::get<std::vector<ListRow>>(e_->value).clear(); active_idx_ = -1; }
    void highlight(int i) {
        auto& rows = std::get<std::vector<ListRow>>(e_->value);
        if (i >= 0 && i < int(rows.size())) rows[i].highlighted = true;
    }
    void remove_highlight(int i) {
        auto& rows = std::get<std::vector<ListRow>>(e_->value);
        if (i >= 0 && i < int(rows.size())) rows[i].highlighted = false;
    }
    void remove(int i) {
        auto& rows = std::get<std::vector<ListRow>>(e_->value);
        if (i >= 0 && i < int(rows.size())) rows.erase(rows.begin() + i);
    }
    void set_active_index(int i) { active_idx_ = i; }
    void set_active(bool a)      { e_->active = a; }
private:
    std::shared_ptr<Element> e_;
    int active_idx_ = -1;
};

// ---------- Container types ----------

struct Panel {
    std::string name;
    bool        is_small = false;
    std::string section;
    std::vector<std::shared_ptr<Element>> elements;

    Checkbox     add_checkbox    (const std::string& name, bool initial,
                                  bool draw_title = true, bool find_protect = false,
                                  bool draw_just_label = false);
    SliderInt    add_slider_int  (const std::string& name, const std::string& postfix,
                                  int value, int min, int max, int step,
                                  bool draw_title = true, bool find_protect = false);
    SliderDouble add_slider_double(const std::string& name, const std::string& postfix,
                                   double value, double min, double max, double step,
                                   bool draw_title = true, bool find_protect = false);
    Input        add_input       (const std::string& name, const std::string& initial,
                                  bool draw_title = true, bool find_protect = false);
    MultiSelect  add_multi_select(const std::string& name, std::vector<std::string> opts,
                                  bool expandable, bool draw_title = true, bool find_protect = false);
    SingleSelect add_single_select(const std::string& name, std::vector<std::string> opts,
                                   int initial_index, bool expandable,
                                   bool draw_title = true, bool find_protect = false);
    Keybind      add_keybind     (const Checkbox& parent, const std::string& name,
                                  int vk, KeybindMode mode,
                                  bool draw_title = true, bool find_protect = false);
    ColorPicker  add_color       (const Checkbox& parent, const std::string& name,
                                  color_t initial, bool find_protect = false);
    Button       add_button      (const std::string& name, std::function<void()> on_click);
    List         add_list        (const std::string& name, std::vector<ListRow> initial,
                                  bool draw_title = true, bool find_protect = false);
};

struct Subtab {
    int         tab_index = 0;
    std::string name;
    std::string section;
    std::vector<std::shared_ptr<Panel>> panels;

    std::shared_ptr<Panel> add_panel(const std::string& name, bool is_small);
    void                   set_active(bool active);
    bool                   active = true;
};

// ---------- Top-level namespace ----------

std::shared_ptr<Subtab> create_subtab(int tab_index, const std::string& name);

// Introspection — returns an Element pointer (of any kind) matching the query
// path, or nullptr. Type string must match ElementKind: "checkbox",
// "slider_int", "slider_double", "input", "multi_select", "single_select",
// "keybind", "color_picker", "button", "list".
std::shared_ptr<Element> find_element(int parent_tab, const std::string& subtab,
                                      const std::string& panel, const std::string& element,
                                      const std::string& type);

// Whether the menu is currently on-screen. Menu code toggles this via
// `set_menu_visible()`.
bool  is_active();
void  set_menu_visible(bool v);

// Serialise / restore every element's value into a text blob. Format is a
// simple line-based `section.tab.subtab.panel.element = value` list — trivial to
// diff, easy to hand-edit, no external dependencies.
std::string construct_config();
void        apply_config(const std::string& blob);

// Cleanup — the engine calls this when a script unloads.
void drop_section(const std::string& section);

// Layout enumeration for the menu renderer.
struct TabView {
    int index = 0;
    std::vector<std::shared_ptr<Subtab>> subtabs;
};
std::vector<TabView> layout();

namespace detail {
    void        push_section(std::string name);
    void        pop_section();
    std::string current_section();
}

} // namespace paris::ui
