// AngelScript variant of esp.lua. The AS surface covers the core UI + render
// stack; for the richer modules (fs, json, net, list, multi_select) use Lua.

Subtab@       esp_tab;
Panel@        esp_panel;
Checkbox@     enabled;
SliderDouble@ thickness;
ColorPicker@  box_color;

void on_reset() {
    enabled.set(false);
    thickness.set(1.5);
    box_color.set(color_t(0, 255, 120));
    engine::log("esp: config reset");
}

int on_frame() {
    if (!enabled.get()) return 1;

    color_t c = box_color.get();
    float   t = float(thickness.get());
    render::draw_rect(vector2(100, 100), vector2(200, 260), c, t);
    render::draw_text(vector2(100, 84), c, "enemy", 20, 2); // font 20, shadow effect
    return 1;
}

int main() {
    @esp_tab   = ui::create_subtab(0, "ESP");
    @esp_panel = esp_tab.add_panel("Player ESP", false);

    @enabled   = esp_panel.add_checkbox("Enable", false);
    @box_color = esp_panel.add_color(enabled, "Box color", color_t(0, 255, 120));
    @thickness = esp_panel.add_slider_double("Box thickness", "px", 1.5, 0.5, 4.0, 0.1);

    esp_panel.add_button("Reset config", @on_reset);
    engine::log("esp (as) loaded for " + engine::get_user_name());
    return 1;
}
