#include "paris/settings.hpp"

#include "paris/callbacks.hpp"

#include <mutex>
#include <sstream>
#include <unordered_map>

namespace paris::settings {

// ---------- Persistence ----------

namespace {
    std::mutex g_mtx;
    Backend    g_backend;
    // In-memory fallback / write-through cache.
    std::unordered_map<std::string, std::string> g_cache;

    std::string prefixed(const std::string& key) {
        auto section = callbacks::detail::current_section();
        if (section.empty()) return key;
        return section + "." + key;
    }
}

void install(Backend b) {
    std::lock_guard lk(g_mtx);
    g_backend = std::move(b);
}

std::string get(const std::string& key, const std::string& def) {
    auto full = prefixed(key);
    std::lock_guard lk(g_mtx);
    auto it = g_cache.find(full);
    if (it != g_cache.end()) return it->second;
    if (g_backend.read) {
        auto s = g_backend.read(full);
        if (!s.empty()) { g_cache[full] = s; return s; }
    }
    return def;
}

void set(const std::string& key, const std::string& v) {
    auto full = prefixed(key);
    std::lock_guard lk(g_mtx);
    g_cache[full] = v;
    if (g_backend.write) g_backend.write(full, v);
}

bool get_bool(const std::string& key, bool def) {
    auto s = get(key, def ? "1" : "0");
    return s == "1" || s == "true";
}
void set_bool(const std::string& key, bool v) { set(key, v ? "1" : "0"); }

double get_number(const std::string& key, double def) {
    auto s = get(key, std::to_string(def));
    try { return std::stod(s); } catch (...) { return def; }
}
void set_number(const std::string& key, double v) {
    std::ostringstream os; os << v; set(key, os.str());
}

color_t get_color(const std::string& key, color_t def) {
    std::ostringstream ds; ds << def.r << "," << def.g << "," << def.b << "," << def.a;
    auto s = get(key, ds.str());
    color_t c = def;
    std::stringstream ss(s); std::string cell; int i = 0;
    while (std::getline(ss, cell, ',')) {
        try {
            int v = std::stoi(cell);
            if      (i == 0) c.r = v;
            else if (i == 1) c.g = v;
            else if (i == 2) c.b = v;
            else if (i == 3) c.a = v;
        } catch (...) {}
        ++i;
    }
    return c;
}
void set_color(const std::string& key, color_t c) {
    std::ostringstream os; os << c.r << "," << c.g << "," << c.b << "," << c.a;
    set(key, os.str());
}

// ---------- Widgets ----------

namespace {
    std::mutex                                 wg_mtx;
    std::string                                wg_section;
    std::vector<std::shared_ptr<Widget>>       wg_current;

    void push(std::shared_ptr<Widget> w) {
        w->section = wg_section;
        std::lock_guard lk(wg_mtx);
        wg_current.push_back(std::move(w));
    }

    std::shared_ptr<Widget> make(WidgetKind k) {
        auto w = std::make_shared<Widget>();
        w->kind = k;
        return w;
    }
}

void create_label(const std::string& text) {
    auto w = make(WidgetKind::Label); w->label = text; push(std::move(w));
}
void create_separator() { push(make(WidgetKind::Separator)); }
void create_spacing(double px) {
    auto w = make(WidgetKind::Spacing); w->px = px; push(std::move(w));
}
void create_checkbox(const std::string& label, const std::string& key) {
    auto w = make(WidgetKind::Checkbox); w->label = label; w->key = key; push(std::move(w));
}
void create_button(const std::string& label, std::function<void()> on_click) {
    auto w = make(WidgetKind::Button); w->label = label; w->on_click = std::move(on_click); push(std::move(w));
}
void create_slider(const std::string& label, const std::string& key,
                   double mn, double mx, double st) {
    auto w = make(WidgetKind::Slider);
    w->label = label; w->key = key; w->d_min = mn; w->d_max = mx; w->d_step = st;
    push(std::move(w));
}
void create_input_text(const std::string& label, const std::string& key) {
    auto w = make(WidgetKind::InputText); w->label = label; w->key = key; push(std::move(w));
}
void create_text_area(const std::string& label, const std::string& key, int lines) {
    auto w = make(WidgetKind::TextArea);
    w->label = label; w->key = key; w->text_area_lines = lines;
    push(std::move(w));
}
void create_dropdown(const std::string& label, const std::string& key,
                     std::vector<std::string> opts) {
    auto w = make(WidgetKind::Dropdown);
    w->label = label; w->key = key; w->options = std::move(opts);
    push(std::move(w));
}
void create_color_picker(const std::string& label, const std::string& key) {
    auto w = make(WidgetKind::ColorPicker); w->label = label; w->key = key; push(std::move(w));
}
void create_keybind(const std::string& label, const std::string& key) {
    auto w = make(WidgetKind::Keybind); w->label = label; w->key = key; push(std::move(w));
}
void create_progress_bar(const std::string& label, double v, double mx) {
    auto w = make(WidgetKind::ProgressBar);
    w->label = label; w->progress_val = v; w->progress_max = mx;
    push(std::move(w));
}

void begin_render(const std::string& section) {
    std::lock_guard lk(wg_mtx);
    wg_section = section;
    wg_current.clear();
}

std::vector<std::shared_ptr<Widget>> take() {
    std::lock_guard lk(wg_mtx);
    auto out = std::move(wg_current);
    wg_current.clear();
    return out;
}

void drop_section(const std::string& section) {
    // Persistence isn't dropped — settings should survive extension reload.
    // Widgets are transient (rebuilt per render), so nothing to clear here.
    (void)section;
}

} // namespace paris::settings
