# ai

Hooks into paris' AI pipeline. Extensions can rewrite prompts before send,
mutate responses, handle tool calls, and inject system prompt context.

## Extension hooks

Define these as top-level functions in your extension — paris auto-wires them.

### `on_ai_before_send(prompt, system_prompt)`

Fires just before a prompt is dispatched. Return a table `{ ok, prompt, system }`
to modify the request. Return `false` to cancel entirely.

```lua
function on_ai_before_send(prompt, system)
    local file = editor.get_active_file()
    return {
        ok = true,
        system = system .. "\n\nCurrent file: " .. file
    }
end
```

### `on_ai_after_response(response)`

Fires after the model returns. Return a string to replace the visible response.

```lua
function on_ai_after_response(response)
    return response:gsub("<%[system]:%s*", "")
end
```

### `on_ai_tool_call(name, args_json)`

Fires when the model calls a registered tool. Return the tool result as a
string; return `""` to let another handler try.

```lua
function on_ai_tool_call(name, args_json)
    if name == "count_lines" then
        return string.format('{"lines": %d}', editor.get_line_count())
    end
    return ""
end
```

### `on_ai_after_tool(name, args_json, result)`

Observability hook — fires after every tool call resolves.

### `on_ai_system_inject()`

Return additional text to append to the system prompt.

```lua
function on_ai_system_inject()
    return "The user prefers concise answers."
end
```

## Introspection

| Function                           | Returns   |
|------------------------------------|-----------|
| `ai.get_active_model()`            | `string`  |
| `ai.set_active_model(name)`        | —         |
| `ai.get_chat_message_count()`      | `int`     |
| `ai.get_chat_message(index)`       | `{ role, content }` |
