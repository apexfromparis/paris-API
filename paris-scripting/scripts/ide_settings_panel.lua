-- @name Settings panel demo
-- @author paris-scripting
-- @version 0.1.0
-- @description Renders a full settings panel with persistent values.

function on_activate()
    -- Initialise defaults the first time the extension runs.
    if settings.get("initialised") == "" then
        settings.set_bool  ("enabled",   true)
        settings.set_number("threshold", 42)
        settings.set       ("greeting",  "hello, paris")
        settings.set       ("initialised", "1")
    end
end

function on_settings_render(x, y, w)
    settings.create_label("Behaviour")
    settings.create_separator()
    settings.create_checkbox   ("Enable feature",  "enabled")
    settings.create_slider     ("Threshold", "threshold", 0, 100, 1)
    settings.create_dropdown   ("Mode", "mode", { "Off", "Passive", "Active" })
    settings.create_spacing(6)

    settings.create_label("Appearance")
    settings.create_separator()
    settings.create_color_picker("Highlight", "highlight_color")
    settings.create_keybind     ("Toggle key", "toggle_key")

    settings.create_spacing(6)
    settings.create_label("Message")
    settings.create_text_area   ("Greeting", "greeting", 3)

    settings.create_button("Reset defaults", function()
        settings.set_bool  ("enabled",   true)
        settings.set_number("threshold", 42)
        settings.set       ("greeting",  "hello, paris")
        editor.show_notification("defaults restored")
    end)

    settings.create_progress_bar("Progress",
                                 settings.get_number("threshold", 0), 100)
end
