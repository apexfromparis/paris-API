# settings

Per-extension key/value persistence and a widget set for the IDE's Settings
panel. Every key is automatically prefixed with the extension's name, so two
extensions can use the same short keys without colliding.

## Persistence

| Function                                       | Returns / effect |
|------------------------------------------------|------------------|
| `settings.get(key, default?)`                  | `string`         |
| `settings.set(key, value)`                     | —                |
| `settings.get_bool(key, default?)`             | `bool`           |
| `settings.set_bool(key, value)`                | —                |
| `settings.get_number(key, default?)`           | `number`         |
| `settings.set_number(key, value)`              | —                |
| `settings.get_color(key, default?)`            | `color_t`        |
| `settings.set_color(key, color)`               | —                |

## Widgets

Call these from `on_settings_render(x, y, w)` — an extension-defined function
paris invokes when your Settings tab is visible. Widgets rebuild every render,
so you can conditionally show / hide them without any bookkeeping.

| Widget                                                             | What it renders |
|--------------------------------------------------------------------|-----------------|
| `settings.create_label(text)`                                      | Static text     |
| `settings.create_separator()`                                      | Horizontal rule |
| `settings.create_spacing(px)`                                      | Vertical spacer |
| `settings.create_checkbox(label, key)`                             | Bool toggle     |
| `settings.create_button(label, callback)`                          | Click button    |
| `settings.create_slider(label, key, min, max, step)`               | Numeric slider  |
| `settings.create_input_text(label, key)`                           | Single line     |
| `settings.create_text_area(label, key, lines)`                     | Multi-line      |
| `settings.create_dropdown(label, key, options)`                    | Combo box       |
| `settings.create_color_picker(label, key)`                         | RGBA picker     |
| `settings.create_keybind(label, key)`                              | Hotkey capture  |
| `settings.create_progress_bar(label, value, max)`                  | Read-only bar   |

Widgets bound to a `key` read + write the underlying setting automatically.
Buttons take a Lua callback fired on click.

## Full example

```lua
function on_activate()
    if settings.get("initialised") == "" then
        settings.set_bool  ("enabled",   true)
        settings.set_number("threshold", 42)
        settings.set       ("initialised", "1")
    end
end

function on_settings_render(x, y, w)
    settings.create_label("Behaviour")
    settings.create_separator()
    settings.create_checkbox("Enable feature", "enabled")
    settings.create_slider  ("Threshold", "threshold", 0, 100, 1)

    settings.create_spacing(6)
    settings.create_label("Appearance")
    settings.create_separator()
    settings.create_color_picker("Highlight", "highlight_color")
    settings.create_keybind     ("Toggle key", "toggle_key")

    settings.create_button("Reset defaults", function()
        settings.set_bool  ("enabled",   true)
        settings.set_number("threshold", 42)
        editor.show_notification("defaults restored")
    end)
end
```
