# tools

Register AI tools the model can call. Combine with [`ai.on_ai_tool_call`](ai.md)
to handle invocations.

## Registration

| Function                                                    | Purpose |
|-------------------------------------------------------------|---------|
| `tools.register(name, description, params_json?)`           | Declare a tool. If `params_json` is empty, build parameters up with `register_param`. |
| `tools.register_param(tool, name, type, description, required?)` | Add a parameter to an existing tool. |
| `tools.unregister(name)`                                    | Remove a tool. |

Parameter types: `"string"`, `"number"`, `"boolean"`, `"object"`, `"array"`.

## Example

```lua
function on_activate()
    tools.register("count_words",
                   "Return the number of words in the currently open file.")

    tools.register("insert_snippet",
                   "Insert a code snippet at the caret.")
    tools.register_param("insert_snippet", "snippet",
                         "string", "The snippet text", true)
end

function on_ai_tool_call(name, args_json)
    if name == "count_words" then
        local n, content = 0, editor.get_active_file_content()
        for _ in content:gmatch("%S+") do n = n + 1 end
        return string.format('{"words": %d}', n)
    end
    if name == "insert_snippet" then
        local args = json.decode(args_json)
        editor.insert_text(args.snippet)
        return '{"ok": true}'
    end
    return ""
end

function on_deactivate()
    tools.unregister("count_words")
    tools.unregister("insert_snippet")
end
```
