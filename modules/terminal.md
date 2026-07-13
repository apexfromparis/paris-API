# terminal

Programmatic access to paris' integrated terminal. Extensions can spawn tabs,
send input, read output.

## Functions

| Function                                              | Returns / effect |
|-------------------------------------------------------|------------------|
| `terminal.create(title?)`                             | `int` — new terminal id |
| `terminal.send_text(id, text)`                        | — |
| `terminal.send_command(id, command)`                  | Convenience: `send_text(id, command .. "\n")` |
| `terminal.read_output(id, max_bytes?)`                | `string` — buffered stdout / stderr |
| `terminal.clear(id)`                                  | Wipe the tab. |
| `terminal.close(id)`                                  | Kill and close. |
| `terminal.set_active(id)` / `terminal.get_active()`   | Which tab is fronted. |
| `terminal.count()`                                    | Number of open terminals. |

`max_bytes` defaults to `4096`.

## Example — build watcher

```lua
local id

function on_activate()
    id = terminal.create("build")
    terminal.send_command(id, "cmake --build build")
end

function on_file_saved(path)
    if not path:match("%.cpp$") and not path:match("%.hpp$") then return end
    terminal.clear(id)
    terminal.send_command(id, "cmake --build build")
    editor.set_status("rebuilding…")
end

function on_deactivate()
    terminal.close(id)
end
```
