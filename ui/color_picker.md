# color\_picker

Full RGBA color picker. Attached to a parent [checkbox](checkbox.md) — same
layout constraint as [keybinds](keybind.md).

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_color(parent_checkbox, name, { r, g, b, a },
                find_protect?)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
ColorPicker@ panel.add_color(Checkbox@ parent,
                             const string &in name,
                             color_t initial);
```
{% endtab %}
{% endtabs %}

| Parameter | Type | Purpose |
|-----------|------|---------|
| `parent`  | Checkbox | Checkbox that hosts the picker. |
| `name`    | string   | Label. |
| initial   | `{r, g, b, a}` (Lua) / `color_t` (AS) | Starting color. Alpha defaults to `255` if omitted. |

## Methods

| Method              | Signature |
|---------------------|-----------|
| `get()`             | `color_t` |
| `set(color)`        | `color_t` |
| `set_active(bool)`  | show/hide |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local enabled   = panel:add_checkbox("Enable ESP", false)
local box_color = panel:add_color(enabled, "Box color", { 0, 255, 120, 255 })

function on_frame()
    if enabled:get() then
        render.draw_rect(vector2.new(100, 100),
                         vector2.new(200, 260),
                         box_color:get(), 2.0)
    end
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Checkbox@    enabled;
ColorPicker@ box_color;

int main()
{
    @enabled   = panel.add_checkbox("Enable ESP", false);
    @box_color = panel.add_color(enabled, "Box color", color_t(0, 255, 120));
    return 1;
}
```
{% endtab %}
{% endtabs %}
