#pragma once

#include "types.hpp"

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

// Extension-scoped settings — a slice of the settings UI + persistent key/value
// store, both namespaced per-extension. Distinct from the game overlay `ui`
// module: settings render into the IDE's Settings panel, not the game menu.

namespace paris::settings {

// ---------- Persistence ----------

using value_t = std::variant<bool, double, std::string, color_t>;

std::string      get       (const std::string& key, const std::string& def = "");
bool             get_bool  (const std::string& key, bool def = false);
double           get_number(const std::string& key, double def = 0.0);
color_t          get_color (const std::string& key, color_t def = { 255, 255, 255, 255 });

void set       (const std::string& key, const std::string& v);
void set_bool  (const std::string& key, bool v);
void set_number(const std::string& key, double v);
void set_color (const std::string& key, color_t v);

// Persistence backend — plug it into whatever storage the host uses (json
// file, registry, sqlite). Not installing anything keeps values in-memory.
struct Backend {
    std::function<std::string(const std::string&)>                 read;
    std::function<void(const std::string&, const std::string&)>    write;
};
void install(Backend b);

// ---------- Widgets ----------

// One widget rendered in the extension's settings panel. Widgets bind to a
// setting key so their live value is stored and restored automatically.

enum class WidgetKind : int {
    Label       = 0,
    Separator   = 1,
    Spacing     = 2,
    Checkbox    = 3,
    Button      = 4,
    Slider      = 5,
    InputText   = 6,
    TextArea    = 7,
    Dropdown    = 8,
    ColorPicker = 9,
    Keybind     = 10,
    ProgressBar = 11,
};

struct Widget {
    WidgetKind  kind;
    std::string label;
    std::string key;   // setting key it's bound to (empty for labels / separators / buttons)
    std::string section; // owning script

    // Slider bounds
    double d_min = 0, d_max = 0, d_step = 1;
    // Spacing / progress
    double px = 0, progress_val = 0, progress_max = 100;
    // Dropdown
    std::vector<std::string> options;
    // Text area lines
    int text_area_lines = 4;

    // Button click handler.
    std::function<void()> on_click;
};

// Widget factories, callable from an extension's `on_settings_render` hook.
void create_label       (const std::string& text);
void create_separator   ();
void create_spacing     (double px);
void create_checkbox    (const std::string& label, const std::string& key);
void create_button      (const std::string& label, std::function<void()> on_click);
void create_slider      (const std::string& label, const std::string& key, double min, double max, double step);
void create_input_text  (const std::string& label, const std::string& key);
void create_text_area   (const std::string& label, const std::string& key, int lines);
void create_dropdown    (const std::string& label, const std::string& key, std::vector<std::string> options);
void create_color_picker(const std::string& label, const std::string& key);
void create_keybind     (const std::string& label, const std::string& key);
void create_progress_bar(const std::string& label, double value, double max);

// Enumerated for the host renderer. Every widget created within the current
// render pass; cleared by `begin_render`.
struct RenderView {
    std::string section;
    std::vector<std::shared_ptr<Widget>> widgets;
};

// Called by the host before invoking `on_settings_render` for a given section.
void begin_render(const std::string& section);
// After the script returns, host takes the collected widgets.
std::vector<std::shared_ptr<Widget>> take();

// Called by the engine on unload.
void drop_section(const std::string& section);

} // namespace paris::settings
