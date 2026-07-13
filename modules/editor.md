# editor

Editor manipulation. Query the current buffer, tabs, cursor, selection; mutate
the buffer, open files, save, navigate. This module is the core surface for
IDE-style extensions.

## Query

| Function                          | Returns           |
|-----------------------------------|-------------------|
| `editor.get_active_file()`        | `string`          |
| `editor.get_active_file_content()`| `string`          |
| `editor.get_active_language()`    | `string` — `"lua"`, `"angelscript"`, etc. |
| `editor.get_root_path()`          | `string` — workspace root |
| `editor.get_selection_text()`     | `string` — currently selected text |
| `editor.get_cursor_line()`        | `int` — 0-based |
| `editor.get_cursor_col()`         | `int` — 0-based |
| `editor.get_line_count()`         | `int` |
| `editor.get_line_text(line)`      | `string` |
| `editor.get_open_files()`         | array of paths |
| `editor.get_tab_count()`          | `int` |
| `editor.get_tab_file(index)`      | `string` — path of the tab at `index` |
| `editor.get_active_tab()`         | `int` |

Empty string / zero means "not available" — no active tab, no backend
installed, etc. Scripts should tolerate the absence.

## Mutate

| Function                                          | Purpose |
|---------------------------------------------------|---------|
| `editor.set_cursor_pos(line, col)`                | Move the caret. |
| `editor.set_selection(sl, sc, el, ec)`            | Select a range. |
| `editor.insert_text(text)`                        | Insert at the caret. |
| `editor.replace_selection(text)`                  | Replace whatever's selected. |
| `editor.open_file(path)`                          | Open a file in a new tab. |
| `editor.save_active_file()`                       | Save. |
| `editor.goto_line(line)`                          | Scroll + move caret to a line. |

## Signals

| Function                             | Purpose |
|--------------------------------------|---------|
| `editor.show_notification(msg)`      | Toast in the IDE. |
| `editor.set_status(msg)`             | Status-bar message. |
| `editor.send_chat_message(msg)`      | Post a message into the AI chat panel. |

## Events (extensions define these as top-level functions)

| Function                             | Fires when |
|--------------------------------------|-----------|
| `on_file_opened(path)`               | A file becomes active or is loaded from disk. |
| `on_file_saved(path)`                | The user saves. |
| `on_buffer_changed(path, line)`      | The buffer content changed on the given line. |
| `on_tab_changed(path)`               | The active tab changed. |

## Example — auto-strip trailing whitespace

```lua
function on_file_saved(path)
    local content = editor.get_active_file_content()
    if content == "" then return end
    local cleaned = content:gsub("[ \t]+\n", "\n")
    if cleaned == content then return end
    editor.set_selection(0, 0, editor.get_line_count(), 0)
    editor.replace_selection(cleaned)
    editor.save_active_file()
end
```
