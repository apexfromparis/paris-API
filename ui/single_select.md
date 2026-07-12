# single\_select

Dropdown that returns the currently-selected index (0-based).

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_single_select(name, options, initial_index, is_expandable,
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
| `initial_index`  | int               | 0-based, defaults to `0`. |
| `is_expandable`  | bool              | Collapsible tree style when true. |

## Methods

| Method              | Signature |
|---------------------|-----------|
| `get()`             | `int` — selected index |
| `set(index)`        | Select programmatically |
| `set_active(bool)`  | show/hide |

## Example

```lua
local style = panel:add_single_select("ESP style",
    { "Corner", "Full", "Skeleton" }, 0, false)

function on_frame()
    local mode = style:get()
    if     mode == 0 then draw_corner_esp()
    elseif mode == 1 then draw_full_esp()
    else                  draw_skeleton_esp() end
    return 1
end
```
