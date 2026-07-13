-- Shows the extended math helpers driving a small animated widget.

local anim_speed
local start_time = os.clock()

function main()
    local subtab = ui.create_subtab(3, "Math")
    local panel  = subtab:add_panel("Anim demo", false)
    anim_speed = panel:add_slider_double("Speed", "x", 1.0, 0.1, 5.0, 0.1)
    engine.log("math_demo loaded")
    return 1
end

function on_frame()
    local t = (os.clock() - start_time) * anim_speed:get()
    local phase = mathx.wrap(t, 0.0, 2.0)
    local ratio = mathx.smoothstep(0.2, 0.8, phase < 1.0 and phase or 2.0 - phase)

    local vp     = render.get_view()
    local center = vector2.new(vp.x * 0.5, vp.y * 0.5)
    local radius = mathx.remap(ratio, 0.0, 1.0, 20.0, 80.0)
    local hue    = mathx.wrap(t * 0.1, 0.0, 1.0)
    local c      = color_t.from_hsv(hue, 0.8, 1.0)

    render.draw_circle_filled(center, radius, c:with_alpha(180), 48)
    render.draw_circle(center, radius + 6, c, 2.0, 48)

    return 1
end
