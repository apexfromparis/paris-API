-- @name Word counter tool
-- @author paris-scripting
-- @version 0.1.0
-- @description Registers an AI tool the model can call to count words in the active file.

function on_activate()
    tools.register("count_words",
                   "Return the number of words in the currently open file.")
    -- No parameters — the model calls it with no args.

    engine.log("word-counter tool registered")
end

-- Fires when the model calls one of the tools we registered.
function on_ai_tool_call(name, args_json)
    if name ~= "count_words" then return "" end
    local content = editor.get_active_file_content()
    local n = 0
    for _ in content:gmatch("%S+") do n = n + 1 end
    return string.format('{"words": %d}', n)
end

-- Optional observation hook — log every tool call for debugging.
function on_ai_after_tool(name, args_json, result)
    engine.log_console(string.format("[ai] %s(%s) -> %s", name, args_json, result))
end

function on_deactivate()
    tools.unregister("count_words")
end
