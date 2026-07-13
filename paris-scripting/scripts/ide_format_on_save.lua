-- @name Format on save
-- @author paris-scripting
-- @version 0.1.0
-- @description Trims trailing whitespace on save.
--
-- IDE-style extension: no main(), just event hooks. Loaded once by paris and
-- kept resident until the user disables it or hot-reloads the file.

function on_activate()
    editor.show_notification("format-on-save loaded")
end

function on_file_saved(path)
    local content = editor.get_active_file_content()
    if content == "" then return end

    -- Strip trailing whitespace from every line.
    local lines = {}
    for line in content:gmatch("([^\n]*)\n?") do
        table.insert(lines, (line:gsub("[ \t]+$", "")))
    end
    -- gmatch with "([^\n]*)\n?" emits a trailing empty match; drop it.
    if lines[#lines] == "" then table.remove(lines) end

    local cleaned = table.concat(lines, "\n")
    if cleaned == content then return end

    editor.set_selection(0, 0, editor.get_line_count(), 0)
    editor.replace_selection(cleaned)
    editor.save_active_file()
    editor.set_status("format-on-save: cleaned " .. path)
end

function on_deactivate()
    engine.log("format-on-save unloading")
end
