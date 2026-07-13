-- ESP-style demo — creates one subtab under tab 0, one panel, a handful of
-- controls, and draws a fake box + label every frame while enabled.

local esp_tab, esp_panel
local enabled, thickness, box_color, style, hotkey

function main()
    esp_tab   = ui.create_subtab(0, "ESP")
    esp_panel = esp_tab:add_panel("Player ESP", false)

    enabled   = esp_panel:add_checkbox("Enable", false)
    hotkey    = esp_panel:add_keybind(enabled, "Toggle key", 0x2D, "toggle") -- Insert
    box_color = esp_panel:add_color(enabled, "Box color", { 0, 255, 120, 255 })

    thickness = esp_panel:add_slider_double("Box thickness", "px", 1.5, 0.5, 4.0, 0.1)
    style     = esp_panel:add_single_select("Style", { "Corner", "Full", "Skeleton" }, 0, false)

    esp_panel:add_button("Reset config", function()
        enabled:set(false)
        thickness:set(1.5)
        box_color:set(color_t.new(0, 255, 120, 255))
        style:set(0)
        engine.log("esp: config reset")
    end)

    engine.log("esp loaded for " .. engine.get_user_name())
    return 1
end

function on_frame()
    if not enabled:get() then return 1 end

    local c  = box_color:get()
    local t  = thickness:get()
    render.draw_rect(vector2.new(100, 100), vector2.new(200, 260), c, t)

    local label = "enemy [" .. style:get() .. "]"
    render.draw_text(vector2.new(100, 84), c, label, render.FONT_20, render.EFFECT_SHADOW)

    return 1
end

function on_unload()
    engine.log("esp unloading")
end
