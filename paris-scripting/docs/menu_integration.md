# Menu integration

The engine keeps a live tree of tabs → subtabs → panels → elements. Your menu
renderer walks that tree each frame via `paris::ui::layout()` and draws
whatever fits your style (ImGui, custom D3D, whatever).

Below is a working ImGui snippet that renders every element kind. Drop it into
your menu tick; adapt the visuals to match your existing style.

```cpp
#include "paris/ui.hpp"
#include <imgui.h>

void draw_paris_menu() {
    using namespace paris::ui;

    if (!ImGui::Begin("paris")) { ImGui::End(); return; }

    if (ImGui::BeginTabBar("root")) {
        for (auto& tab : layout()) {
            std::string tab_name = "Tab " + std::to_string(tab.index);
            if (!ImGui::BeginTabItem(tab_name.c_str())) continue;

            if (ImGui::BeginTabBar("subtabs")) {
                for (auto& sub : tab.subtabs) {
                    if (!sub->active) continue;
                    if (!ImGui::BeginTabItem(sub->name.c_str())) continue;

                    for (auto& p : sub->panels) {
                        if (ImGui::CollapsingHeader(p->name.c_str(),
                                ImGuiTreeNodeFlags_DefaultOpen)) {
                            for (auto& el : p->elements) draw_element(el);
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void draw_element(std::shared_ptr<Element> e) {
    if (!e->active) return;

    switch (e->kind) {
        case ElementKind::Checkbox: {
            bool v = std::get<bool>(e->value);
            if (ImGui::Checkbox(e->name.c_str(), &v)) e->value = v;
            for (auto& child : e->children) draw_element(child);
            break;
        }
        case ElementKind::SliderInt: {
            int v = std::get<int>(e->value);
            if (ImGui::SliderInt(e->name.c_str(), &v, e->i_min, e->i_max)) e->value = v;
            break;
        }
        case ElementKind::SliderDouble: {
            double v = std::get<double>(e->value);
            float  fv = float(v);
            if (ImGui::SliderFloat(e->name.c_str(), &fv, float(e->d_min), float(e->d_max)))
                e->value = double(fv);
            break;
        }
        case ElementKind::Input: {
            auto& s = std::get<std::string>(e->value);
            char buf[512]; std::snprintf(buf, sizeof(buf), "%s", s.c_str());
            if (ImGui::InputText(e->name.c_str(), buf, sizeof(buf))) e->value = std::string(buf);
            break;
        }
        case ElementKind::SingleSelect: {
            int v = std::get<int>(e->value);
            std::vector<const char*> items;
            for (auto& s : e->options) items.push_back(s.c_str());
            if (ImGui::Combo(e->name.c_str(), &v, items.data(), int(items.size())))
                e->value = v;
            break;
        }
        case ElementKind::MultiSelect: {
            auto& mask = std::get<std::vector<bool>>(e->value);
            if (ImGui::TreeNode(e->name.c_str())) {
                for (size_t i = 0; i < e->options.size(); ++i) {
                    bool v = mask[i];
                    if (ImGui::Checkbox(e->options[i].c_str(), &v)) mask[i] = v;
                }
                ImGui::TreePop();
            }
            break;
        }
        case ElementKind::ColorPicker: {
            auto c = std::get<color_t>(e->value);
            float col[4] = { c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f };
            if (ImGui::ColorEdit4(e->name.c_str(), col))
                e->value = color_t{ int(col[0]*255), int(col[1]*255),
                                    int(col[2]*255), int(col[3]*255) };
            break;
        }
        case ElementKind::Keybind: {
            ImGui::Text("%s: %s", e->name.c_str(),
                paris::input::keyboard::vk_name(e->keybind_vk).c_str());
            // Your capture UI here — set e->keybind_vk / e->keybind_mode
            // when the user rebinds.
            update_keybind_state(e); // implement below
            break;
        }
        case ElementKind::Button:
            if (ImGui::Button(e->name.c_str()) && e->on_click) e->on_click();
            break;
        case ElementKind::List: {
            auto& rows = std::get<std::vector<ListRow>>(e->value);
            ImGui::Text("%s", e->name.c_str());
            if (ImGui::BeginListBox(("##" + e->name).c_str())) {
                for (auto& r : rows) {
                    if (r.highlighted) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 80, 255));
                    ImGui::Selectable((r.name + " — " + r.info).c_str(), false);
                    if (r.highlighted) ImGui::PopStyleColor();
                }
                ImGui::EndListBox();
            }
            break;
        }
    }
}
```

## Keybind runtime

The engine doesn't tick keybind states itself — plug it into your input pipeline
and update `Element::keybind_state` per frame. A rough shape:

```cpp
void update_keybind_state(std::shared_ptr<paris::ui::Element> e) {
    using paris::ui::KeybindMode;
    bool held = paris::input::keyboard::is_down(e->keybind_vk);
    bool pressed = paris::input::keyboard::is_pressed(e->keybind_vk);

    switch (e->keybind_mode) {
        case KeybindMode::Off:      e->keybind_state = false; break;
        case KeybindMode::AlwaysOn: e->keybind_state = true;  break;
        case KeybindMode::On:       e->keybind_state = held;  break;
        case KeybindMode::Single:   e->keybind_state = pressed; break;
        case KeybindMode::Toggle:
            if (pressed) e->keybind_state = !e->keybind_state;
            break;
    }

    // Perception's convention: the keybind gates its parent checkbox. If the
    // parent is a checkbox, mirror the keybind state onto it whenever a
    // keybind is present.
    if (e->parent && e->parent->kind == paris::ui::ElementKind::Checkbox) {
        // Only override when the keybind is in a mode that expresses intent.
        if (e->keybind_mode == KeybindMode::On ||
            e->keybind_mode == KeybindMode::Toggle ||
            e->keybind_mode == KeybindMode::AlwaysOn) {
            e->parent->value = e->keybind_state;
        }
    }
}
```

## Hot reload

`ScriptEngine::load_file(path)` will discard the previous version of a script
before loading the new one — this includes running its `on_unload`, dropping
its UI subtree, and its callbacks. Wire that call into a `ReadDirectoryChangesW`
watcher on your `scripts/` directory and edits become live without restarting
the host.
