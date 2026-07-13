-- @name Config Demo
-- @author paris-scripting
-- @version 0.1.0
-- @description Shows metadata headers and JSON round-trip

local subtab, panel
local fov, colors, style

function main()
    subtab = ui.create_subtab(3, "Config")
    panel  = subtab:add_panel("Preset", false)

    fov    = panel:add_slider_double("FOV", "°", 90.0, 40.0, 120.0, 0.5)
    style  = panel:add_single_select("Style", { "Classic", "Modern", "Neon" }, 0, false)

    panel:add_button("Export JSON", function()
        local preset = {
            fov   = fov:get(),
            style = style:get(),
            colors = { { name = "primary",  rgb = { 80, 200, 255 } },
                       { name = "accent",   rgb = { 255, 120, 80 } } }
        }
        local encoded = json.encode(preset)
        fs.create_directory("configs")
        fs.create_file("configs/preset.json", encoded)
        engine.log("wrote configs/preset.json (" .. #encoded .. " bytes)")
    end)

    panel:add_button("Import JSON", function()
        if not fs.does_file_exist("configs/preset.json") then
            engine.log_error("no preset saved yet")
            return
        end
        local ok, raw = fs.read_file("configs/preset.json")
        if not ok then return end
        local decoded = json.decode(raw)
        if not decoded then engine.log_error("bad json"); return end
        fov:set(decoded.fov)
        style:set(decoded.style)
        engine.log("loaded preset, " .. #decoded.colors .. " colors")
    end)

    return 1
end

function on_frame() return 1 end
