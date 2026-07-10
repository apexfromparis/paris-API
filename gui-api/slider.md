---
description: A numeric slider between a min and max.
---

# slider

Adds a draggable numeric slider. Use `gui.slider` for integers and
`gui.slider_float` for decimals.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
gui.slider(label, default, min, max) -> Slider          -- int
gui.slider_float(label, default, min, max) -> Slider    -- float
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Slider@ gui::slider(const string &in label, int default, int min, int max)
Slider@ gui::slider_float(const string &in label, float default, float min, float max)
```
{% endtab %}
{% endtabs %}

## Parameters

| Parameter | Type          | Description                    |
| --------- | ------------- | ----------------------------- |
| `label`   | `string`      | Text shown above the slider.  |
| `default` | `int`/`float` | Initial value.                |
| `min`     | `int`/`float` | Lowest selectable value.      |
| `max`     | `int`/`float` | Highest selectable value.     |

## Handle methods

| Method        | Returns       | Description        |
| ------------- | ------------- | ------------------ |
| `get()`       | `int`/`float` | Current value.     |
| `set(value)`  | —             | Force the value.   |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local thickness = gui.slider("Box thickness", 2, 1, 10)
local smoothing = gui.slider_float("Smoothing", 0.5, 0.0, 1.0)

local function on_paint()
    draw_box(thickness:get(), smoothing:get())
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Slider@ thickness = gui::slider("Box thickness", 2, 1, 10);
Slider@ smoothing = gui::slider_float("Smoothing", 0.5f, 0.0f, 1.0f);

void on_paint()
{
    draw_box(thickness.get(), smoothing.get());
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}
