---
description: A dropdown of named options.
---

# combo

Adds a dropdown. The value is the **index** of the selected item (0-based).

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
gui.combo(label: string, items: table) -> Combo
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Combo@ gui::combo(const string &in label, array<string> items)
```
{% endtab %}
{% endtabs %}

## Parameters

| Parameter | Type              | Description                     |
| --------- | ----------------- | ------------------------------- |
| `label`   | `string`          | Text shown above the dropdown.  |
| `items`   | `string[]`        | The selectable options.         |

## Handle methods

| Method        | Returns  | Description                          |
| ------------- | -------- | ------------------------------------ |
| `get()`       | `int`    | Index of the selected item (0-based).|
| `set(index)`  | —        | Select an item by index.             |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local mode = gui.combo("ESP mode", { "Off", "Box", "Corner" })

local function on_paint()
    local selected = mode:get()
    if selected == 1 then
        draw_box()
    elseif selected == 2 then
        draw_corner()
    end
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
array<string> modes = { "Off", "Box", "Corner" };
Combo@ mode = gui::combo("ESP mode", modes);

void on_paint()
{
    int selected = mode.get();
    if (selected == 1)
        draw_box();
    else if (selected == 2)
        draw_corner();
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}
