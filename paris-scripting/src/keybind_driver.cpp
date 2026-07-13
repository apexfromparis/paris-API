#include "paris/keybind_driver.hpp"

#include "paris/input.hpp"
#include "paris/ui.hpp"

namespace paris::keybind_driver {

namespace {
    void update_one(std::shared_ptr<ui::Element> e) {
        using ui::KeybindMode;
        bool held    = input::keyboard::is_down(e->keybind_vk);
        bool pressed = input::keyboard::is_pressed(e->keybind_vk);

        switch (e->keybind_mode) {
            case KeybindMode::Off:      e->keybind_state = false; break;
            case KeybindMode::AlwaysOn: e->keybind_state = true;  break;
            case KeybindMode::On:       e->keybind_state = held;  break;
            case KeybindMode::Single:   e->keybind_state = pressed; break;
            case KeybindMode::Toggle:
                if (pressed) e->keybind_state = !e->keybind_state;
                break;
        }

        // Mirror to the parent checkbox for the modes that express intent.
        if (e->parent && e->parent->kind == ui::ElementKind::Checkbox) {
            if (e->keybind_mode == KeybindMode::On ||
                e->keybind_mode == KeybindMode::Toggle ||
                e->keybind_mode == KeybindMode::AlwaysOn) {
                e->parent->value = e->keybind_state;
            }
        }
    }
}

void tick() {
    for (auto& tab : ui::layout()) {
        for (auto& sub : tab.subtabs) {
            if (!sub->active) continue;
            for (auto& panel : sub->panels) {
                for (auto& el : panel->elements) {
                    for (auto& child : el->children) {
                        if (child->kind == ui::ElementKind::Keybind) update_one(child);
                    }
                }
            }
        }
    }
}

} // namespace paris::keybind_driver
