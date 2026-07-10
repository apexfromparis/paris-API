---
description: A color picker whose value is a color_t.
---

# color\_picker

Adds a color picker. Its value is a [`color_t`](../types/color_t.md).

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
gui.color_picker(label: string, default: color_t) -> ColorPicker
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
ColorPicker@ gui::color_picker(const string &in label, color_t default)
```
{% endtab %}
{% endtabs %}

## Parameters

| Parameter | Type      | Description                    |
| --------- | --------- | ----------------------------- |
| `label`   | `string`  | Text shown next to the swatch.|
| `default` | `color_t` | Initial color.                |

## Handle methods

| Method        | Returns   | Description        |
| ------------- | --------- | ------------------ |
| `get()`       | `color_t` | Current color.     |
| `set(color)`  | —         | Force the color.   |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local box_color = gui.color_picker("Box color", color_t.new(0, 255, 120))

local function on_paint()
    render.text(vec2_t.new(10, 10), box_color:get(), "colored text")
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
ColorPicker@ box_color = gui::color_picker("Box color", color_t(0, 255, 120));

void on_paint()
{
    render::text(vec2_t(10, 10), box_color.get(), "colored text");
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}
