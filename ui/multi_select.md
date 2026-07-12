# multi\_select

A checklist. Each option can be independently on or off.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_multi_select(name, options, is_expandable,
                       draw_title?, find_protect?)
```
{% endtab %}
{% tab title="AngelScript" %}
Lua only for now.
{% endtab %}
{% endtabs %}

| Parameter        | Type              | Purpose |
|------------------|-------------------|---------|
| `name`           | string            | Label. |
| `options`        | table of strings  | Choices, in display order. |
| `is_expandable`  | bool              | Render as a collapsible tree when true. |
| `draw_title`     | bool              | Hide label when `false`. |
| `find_protect`   | bool              | Skip in config serialisation. |

## Methods

| Method                  | Signature |
|-------------------------|-----------|
| `get()`                 | table of bools, in the same order as `options` |
| `set(index, bool)`      | Toggle a single entry (1-indexed in Lua) |
| `set_active(bool)`      | show/hide |

## Example

```lua
local visuals = panel:add_multi_select("Draw",
    { "Box", "Name", "Health bar", "Skeleton" }, false)

function on_frame()
    local flags = visuals:get()
    if flags[1] then draw_box() end
    if flags[2] then draw_name() end
    if flags[3] then draw_health() end
    if flags[4] then draw_skeleton() end
    return 1
end
```
