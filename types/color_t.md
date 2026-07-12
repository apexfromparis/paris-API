# color\_t

RGBA color, one byte per channel (0–255).

## Fields

| Name | Type | Purpose      |
|------|------|--------------|
| `r`  | int  | Red          |
| `g`  | int  | Green        |
| `b`  | int  | Blue         |
| `a`  | int  | Alpha        |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local c1 = color_t.new(255, 0, 0)       -- opaque red
local c2 = color_t.new(0, 255, 0, 128)  -- half-transparent green
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
color_t c1(255, 0, 0);        // opaque red
color_t c2(0, 255, 0, 128);   // half-transparent green
```
{% endtab %}
{% endtabs %}

Alpha defaults to `255` (fully opaque).

## Methods

| Method              | Returns   | Purpose |
|---------------------|-----------|---------|
| `with_alpha(a)`     | `color_t` | Return a copy with the alpha channel replaced. |
| `lerp(other, t)`    | `color_t` | Linear interpolation between two colors. `t` is clamped to `[0, 1]`. |

## Statics

| Function                | Returns   | Purpose |
|-------------------------|-----------|---------|
| `color_t.from_hsv(h, s, v)` (Lua) / `color_t::from_hsv(h, s, v)` (AS) | `color_t` | Build a color from hue / saturation / value in `[0, 1]`. Great for rainbow effects. |

## Examples

{% tabs %}
{% tab title="Lua" %}
```lua
-- health-colored bar
local col = color_t.new(255, 0, 0):lerp(color_t.new(0, 255, 0), health_pct)
render.draw_rect_filled(vector2.new(20, 20),
                        vector2.new(120, 30),
                        col:with_alpha(180))

-- rainbow border
local t = os.clock() % 1.0
render.draw_rect(vector2.new(20, 40),
                 vector2.new(200, 60),
                 color_t.from_hsv(t, 0.8, 1.0), 2.0)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
color_t col = color_t(255, 0, 0).lerp(color_t(0, 255, 0), health_pct);
render::draw_rect_filled(vector2(20, 20),
                         vector2(120, 30),
                         col.with_alpha(180));
```
{% endtab %}
{% endtabs %}
