#include "paris/ui.hpp"

#include <algorithm>
#include <array>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace paris::ui {

namespace {
    constexpr int kMaxTabs = 5;

    std::mutex g_mtx;
    std::array<std::vector<std::shared_ptr<Subtab>>, kMaxTabs> g_tabs;
    bool g_menu_visible = false;

    thread_local std::vector<std::string> tl_section_stack;

    std::string current_section_or_default() {
        return tl_section_stack.empty() ? std::string{"anonymous"} : tl_section_stack.back();
    }

    // Central attach — sets the section tag and pushes onto the panel.
    void attach(Panel* panel, const std::shared_ptr<Element>& el) {
        el->section = current_section_or_default();
        panel->elements.push_back(el);
    }
}

namespace detail {
    void        push_section(std::string name) { tl_section_stack.push_back(std::move(name)); }
    void        pop_section()                  { if (!tl_section_stack.empty()) tl_section_stack.pop_back(); }
    std::string current_section()              { return current_section_or_default(); }
}

// ---------- Panel::add_* ----------

Checkbox Panel::add_checkbox(const std::string& n, bool init,
                             bool title, bool fp, bool label_only) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::Checkbox;
    e->name = n; e->draw_title = title; e->find_protect = fp; e->draw_just_label = label_only;
    e->value = init;
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return Checkbox(e);
}

SliderInt Panel::add_slider_int(const std::string& n, const std::string& post,
                                int v, int mn, int mx, int st, bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::SliderInt;
    e->name = n; e->postfix = post; e->draw_title = title; e->find_protect = fp;
    e->i_min = mn; e->i_max = mx; e->i_step = st;
    e->value = v;
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return SliderInt(e);
}

SliderDouble Panel::add_slider_double(const std::string& n, const std::string& post,
                                      double v, double mn, double mx, double st,
                                      bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::SliderDouble;
    e->name = n; e->postfix = post; e->draw_title = title; e->find_protect = fp;
    e->d_min = mn; e->d_max = mx; e->d_step = st;
    e->value = v;
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return SliderDouble(e);
}

Input Panel::add_input(const std::string& n, const std::string& init,
                       bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::Input;
    e->name = n; e->draw_title = title; e->find_protect = fp;
    e->value = init;
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return Input(e);
}

MultiSelect Panel::add_multi_select(const std::string& n, std::vector<std::string> opts,
                                    bool /*expandable*/, bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::MultiSelect;
    e->name = n; e->draw_title = title; e->find_protect = fp;
    e->options = std::move(opts);
    e->value = std::vector<bool>(e->options.size(), false);
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return MultiSelect(e);
}

SingleSelect Panel::add_single_select(const std::string& n, std::vector<std::string> opts,
                                      int init, bool /*expandable*/, bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::SingleSelect;
    e->name = n; e->draw_title = title; e->find_protect = fp;
    e->options = std::move(opts);
    e->value = init;
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return SingleSelect(e);
}

Keybind Panel::add_keybind(const Checkbox& parent, const std::string& n,
                           int vk, KeybindMode mode, bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::Keybind;
    e->name = n; e->draw_title = title; e->find_protect = fp;
    e->keybind_vk = vk; e->keybind_mode = mode;
    e->value = 0;
    std::lock_guard lk(g_mtx);
    auto parent_el = parent.raw();
    e->parent = parent_el.get();
    parent_el->children.push_back(e);
    e->section = current_section_or_default();
    return Keybind(e);
}

ColorPicker Panel::add_color(const Checkbox& parent, const std::string& n,
                             color_t init, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::ColorPicker;
    e->name = n; e->find_protect = fp;
    e->value = init;
    std::lock_guard lk(g_mtx);
    auto parent_el = parent.raw();
    e->parent = parent_el.get();
    parent_el->children.push_back(e);
    e->section = current_section_or_default();
    return ColorPicker(e);
}

Button Panel::add_button(const std::string& n, std::function<void()> on_click) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::Button;
    e->name = n;
    e->on_click = std::move(on_click);
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return Button(e);
}

List Panel::add_list(const std::string& n, std::vector<ListRow> init,
                     bool title, bool fp) {
    auto e = std::make_shared<Element>();
    e->kind = ElementKind::List;
    e->name = n; e->draw_title = title; e->find_protect = fp;
    e->value = std::move(init);
    std::lock_guard lk(g_mtx);
    attach(this, e);
    return List(e);
}

// ---------- Subtab ----------

std::shared_ptr<Panel> Subtab::add_panel(const std::string& n, bool small) {
    auto p = std::make_shared<Panel>();
    p->name = n; p->is_small = small; p->section = current_section_or_default();
    std::lock_guard lk(g_mtx);
    panels.push_back(p);
    return p;
}

void Subtab::set_active(bool a) { active = a; }

// ---------- Top-level ----------

std::shared_ptr<Subtab> create_subtab(int tab_index, const std::string& name) {
    if (tab_index < 0 || tab_index >= kMaxTabs) return nullptr;
    auto s = std::make_shared<Subtab>();
    s->tab_index = tab_index; s->name = name; s->section = current_section_or_default();
    std::lock_guard lk(g_mtx);
    g_tabs[tab_index].push_back(s);
    return s;
}

std::shared_ptr<Element> find_element(int parent_tab, const std::string& subtab,
                                      const std::string& panel, const std::string& element,
                                      const std::string& type) {
    static const std::unordered_map<std::string, ElementKind> types = {
        { "checkbox",      ElementKind::Checkbox },
        { "slider_int",    ElementKind::SliderInt },
        { "slider_double", ElementKind::SliderDouble },
        { "input",         ElementKind::Input },
        { "multi_select",  ElementKind::MultiSelect },
        { "single_select", ElementKind::SingleSelect },
        { "keybind",       ElementKind::Keybind },
        { "color_picker",  ElementKind::ColorPicker },
        { "button",        ElementKind::Button },
        { "list",          ElementKind::List },
    };
    auto tk = types.find(type);
    if (tk == types.end()) return nullptr;
    if (parent_tab < 0 || parent_tab >= kMaxTabs) return nullptr;

    std::lock_guard lk(g_mtx);
    for (auto& st : g_tabs[parent_tab]) {
        if (st->name != subtab) continue;
        for (auto& p : st->panels) {
            if (p->name != panel) continue;
            for (auto& el : p->elements) {
                if (el->name == element && el->kind == tk->second) return el;
                for (auto& child : el->children) {
                    if (child->name == element && child->kind == tk->second) return child;
                }
            }
        }
    }
    return nullptr;
}

bool is_active()              { std::lock_guard lk(g_mtx); return g_menu_visible; }
void set_menu_visible(bool v) { std::lock_guard lk(g_mtx); g_menu_visible = v; }

std::vector<TabView> layout() {
    std::lock_guard lk(g_mtx);
    std::vector<TabView> out;
    for (int i = 0; i < kMaxTabs; ++i) {
        if (g_tabs[i].empty()) continue;
        out.push_back({ i, g_tabs[i] });
    }
    return out;
}

void drop_section(const std::string& section) {
    std::lock_guard lk(g_mtx);
    for (int i = 0; i < kMaxTabs; ++i) {
        auto& subs = g_tabs[i];
        subs.erase(std::remove_if(subs.begin(), subs.end(),
            [&](const std::shared_ptr<Subtab>& s){ return s->section == section; }), subs.end());
        for (auto& s : subs) {
            s->panels.erase(std::remove_if(s->panels.begin(), s->panels.end(),
                [&](const std::shared_ptr<Panel>& p){ return p->section == section; }), s->panels.end());
            for (auto& p : s->panels) {
                p->elements.erase(std::remove_if(p->elements.begin(), p->elements.end(),
                    [&](const std::shared_ptr<Element>& e){ return e->section == section; }), p->elements.end());
            }
        }
    }
}

// ---------- Config serialisation ----------

namespace {
    void write_value(std::ostringstream& os, const Element& e) {
        switch (e.kind) {
            case ElementKind::Checkbox:     os << (std::get<bool>(e.value) ? "1" : "0"); break;
            case ElementKind::SliderInt:    os << std::get<int>(e.value); break;
            case ElementKind::SliderDouble: os << std::get<double>(e.value); break;
            case ElementKind::Input:        os << '"' << std::get<std::string>(e.value) << '"'; break;
            case ElementKind::MultiSelect: {
                const auto& v = std::get<std::vector<bool>>(e.value);
                for (size_t i = 0; i < v.size(); ++i) { if (i) os << ','; os << (v[i] ? '1' : '0'); }
                break;
            }
            case ElementKind::SingleSelect: os << std::get<int>(e.value); break;
            case ElementKind::Keybind:      os << e.keybind_vk << ',' << int(e.keybind_mode); break;
            case ElementKind::ColorPicker: {
                auto c = std::get<color_t>(e.value);
                os << c.r << ',' << c.g << ',' << c.b << ',' << c.a;
                break;
            }
            default: break;
        }
    }

    void parse_value(const std::string& raw, Element& e) {
        try {
            switch (e.kind) {
                case ElementKind::Checkbox:     e.value = (raw == "1"); break;
                case ElementKind::SliderInt:    e.value = std::stoi(raw); break;
                case ElementKind::SliderDouble: e.value = std::stod(raw); break;
                case ElementKind::Input: {
                    if (raw.size() >= 2 && raw.front()=='"' && raw.back()=='"')
                        e.value = raw.substr(1, raw.size()-2);
                    else e.value = raw;
                    break;
                }
                case ElementKind::MultiSelect: {
                    std::vector<bool> v;
                    std::stringstream ss(raw); std::string cell;
                    while (std::getline(ss, cell, ',')) v.push_back(cell == "1");
                    e.value = std::move(v);
                    break;
                }
                case ElementKind::SingleSelect: e.value = std::stoi(raw); break;
                case ElementKind::Keybind: {
                    auto comma = raw.find(',');
                    if (comma == std::string::npos) break;
                    e.keybind_vk   = std::stoi(raw.substr(0, comma));
                    e.keybind_mode = KeybindMode(std::stoi(raw.substr(comma + 1)));
                    break;
                }
                case ElementKind::ColorPicker: {
                    color_t c; std::stringstream ss(raw); std::string cell; int i = 0;
                    while (std::getline(ss, cell, ',')) {
                        int v = std::stoi(cell);
                        if      (i == 0) c.r = v;
                        else if (i == 1) c.g = v;
                        else if (i == 2) c.b = v;
                        else if (i == 3) c.a = v;
                        ++i;
                    }
                    e.value = c;
                    break;
                }
                default: break;
            }
        } catch (...) { /* malformed — silently skip */ }
    }
}

std::string construct_config() {
    std::lock_guard lk(g_mtx);
    std::ostringstream os;
    for (int i = 0; i < kMaxTabs; ++i) {
        for (auto& st : g_tabs[i]) {
            for (auto& p : st->panels) {
                for (auto& el : p->elements) {
                    os << i << '.' << st->name << '.' << p->name << '.' << el->name << " = ";
                    write_value(os, *el);
                    os << '\n';
                    for (auto& child : el->children) {
                        os << i << '.' << st->name << '.' << p->name << '.' << el->name
                           << '/' << child->name << " = ";
                        write_value(os, *child);
                        os << '\n';
                    }
                }
            }
        }
    }
    return os.str();
}

void apply_config(const std::string& blob) {
    std::lock_guard lk(g_mtx);
    std::stringstream in(blob);
    std::string line;
    while (std::getline(in, line)) {
        auto eq = line.find(" = ");
        if (eq == std::string::npos) continue;
        auto path = line.substr(0, eq);
        auto val  = line.substr(eq + 3);
        auto d1 = path.find('.'); if (d1 == std::string::npos) continue;
        auto d2 = path.find('.', d1 + 1); if (d2 == std::string::npos) continue;
        auto d3 = path.find('.', d2 + 1); if (d3 == std::string::npos) continue;
        int  tab_idx;
        try { tab_idx = std::stoi(path.substr(0, d1)); } catch (...) { continue; }
        auto subtab   = path.substr(d1 + 1, d2 - d1 - 1);
        auto panel    = path.substr(d2 + 1, d3 - d2 - 1);
        auto rest     = path.substr(d3 + 1);
        std::string element = rest, child;
        auto slash = rest.find('/');
        if (slash != std::string::npos) { element = rest.substr(0, slash); child = rest.substr(slash + 1); }
        if (tab_idx < 0 || tab_idx >= kMaxTabs) continue;
        for (auto& st : g_tabs[tab_idx]) {
            if (st->name != subtab) continue;
            for (auto& p : st->panels) {
                if (p->name != panel) continue;
                for (auto& el : p->elements) {
                    if (el->name != element) continue;
                    if (child.empty()) parse_value(val, *el);
                    else for (auto& c : el->children) if (c->name == child) parse_value(val, *c);
                }
            }
        }
    }
}

} // namespace paris::ui
