-- Reads live input state and prints the mouse position + any typed text.
-- Handy for debugging your input pipeline before layering real features on top.

local subtab, panel, hover_zone

function main()
    subtab = ui.create_subtab(1, "Debug")
    panel  = subtab:add_panel("Input", true)

    hover_zone = panel:add_checkbox("Show hover zone", true)

    engine.log("input_demo loaded")
    return 1
end

function on_frame()
    local mp = input.mouse.get_position()
    local vp = render.get_view()

    -- Overlay corner readouts.
    render.draw_text(vector2.new(10, 10),
                     color_t.new(255, 255, 255, 220),
                     string.format("mouse (%.0f, %.0f)  fps %.0f", mp.x, mp.y, render.get_fps()),
                     render.FONT_18)

    if hover_zone:get() then
        local mn = vector2.new(vp.x * 0.4, vp.y * 0.4)
        local mx = vector2.new(vp.x * 0.6, vp.y * 0.6)
        local hovered = input.mouse.hover_rect(mn, mx)
        local col = hovered
            and color_t.new(80, 200, 255, 60)
            or  color_t.new(255, 255, 255, 30)
        render.draw_rect_filled(mn, mx, col, 6.0)
    end

    -- Held keys, comma-separated.
    local keys = input.keyboard.get_pressed_keys()
    if #keys > 0 then
        local names = {}
        for i, vk in ipairs(keys) do names[i] = input.keyboard.vk_name(vk) end
        render.draw_text(vector2.new(10, 32),
                         color_t.new(255, 200, 80, 255),
                         "keys: " .. table.concat(names, ", "),
                         render.FONT_18)
    end

    -- Recent typed text (buffered by the host WndProc).
    local typed = input.text.get_recent()
    if typed and #typed > 0 then
        engine.log_console("typed: " .. typed)
    end

    return 1
end
