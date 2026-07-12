# checkbox

A boolean toggle. Also acts as the anchor for [keybinds](keybind.md) and
[color pickers](color_picker.md) — attach up to two of each to the same
checkbox.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_checkbox(name, initial,
                   draw_title?, find_protect?, draw_just_label?)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Checkbox@ panel.add_checkbox(const string &in name, bool initial);
```
{% endtab %}
{% endtabs %}

| Parameter          | Type    | Default | Purpose |
|--------------------|---------|---------|---------|
| `name`             | string  | —       | Label shown next to the box. |
| `initial`          | bool    | —       | Starting value. |
| `draw_title`       | bool    | `true`  | Hide the label when `false` (checkbox only). |
| `find_protect`     | bool    | `false` | Skip this element when serialising the UI to a config string. |
| `draw_just_label`  | bool    | `false` | Render as a plain label, no box. |

## Methods

| Method             | Signature |
|--------------------|-----------|
| `get()`            | `bool` |
| `set(value)`       | `bool` |
| `set_active(bool)` | show/hide |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local enabled = panel:add_checkbox("Enable ESP", false)

function on_frame()
    if enabled:get() then
        -- draw ESP
    end
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Checkbox@ enabled;

int main()
{
    @enabled = panel.add_checkbox("Enable ESP", false);
    return 1;
}

int on_frame()
{
    if (enabled.get()) { /* draw ESP */ }
    return 1;
}
```
{% endtab %}
{% endtabs %}
