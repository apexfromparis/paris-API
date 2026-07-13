-- Demonstrates the net + json + fs modules together — fetch a JSON endpoint,
-- decode it, cache the raw body to disk, and surface a status line in the UI.

local status

function main()
    local subtab = ui.create_subtab(2, "Net")
    local panel  = subtab:add_panel("Net demo", true)
    status = panel:add_input("Last status", "idle", false)

    panel:add_button("Fetch time", function()
        local ok, code, body = net.http_get("https://worldtimeapi.org/api/timezone/Etc/UTC", 5000)
        if not ok then
            status:set("http failed")
            engine.log_error("net_demo: http failed")
            return
        end

        local parsed = json.decode(body)
        if not parsed then
            status:set("json parse failed (" .. code .. ")")
            return
        end

        -- Cache the raw body — sandboxed under Documents\My Games\paris.
        fs.create_directory("cache")
        fs.create_file("cache/last_time.json", body)

        status:set("OK " .. code)
        engine.log("net_demo: fetched " .. #body .. " bytes")
    end)

    return 1
end

function on_frame() return 1 end
