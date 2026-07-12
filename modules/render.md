# render

Immediate-mode drawing API. Coordinates are in overlay pixels — `(0, 0)` is
the top-left corner, `render.get_view()` gives you the size.

## Viewport

| Function                | Returns    | Purpose |
|-------------------------|------------|---------|
| `render.get_view()`     | `vector2`  | Viewport size in overlay pixels. |
| `render.get_view_scale()`| `float`   | DPI multiplier (`1.0` = 100%). |
| `render.get_fps()`      | `float`    | Current frame rate. |

## Shapes

| Function | Signature |
|----------|-----------|
| `draw_rect`        | `(min, max, color, thickness?, rounding?)` |
| `draw_rect_filled` | `(min, max, color, rounding?)` |
| `draw_gradient`    | `(min, max, top_left, top_right, bottom_right, bottom_left)` |
| `draw_line`        | `(a, b, color, thickness?)` |
| `draw_circle`      | `(center, radius, color, thickness?, segments?)` |
| `draw_circle_filled`| `(center, radius, color, segments?)` |
| `draw_arc`         | `(center, radius, from_rad, to_rad, color, thickness?, segments?)` |
| `draw_triangle`    | `(a, b, c, color, thickness?)` |
| `draw_triangle_filled` | `(a, b, c, color)` |
| `draw_polygon`     | `({points}, color, thickness?)` |
| `draw_polygon_filled` | `({points}, color)` |

Defaults: `thickness = 1.0`, `rounding = 0.0`, `segments = 24`.

## Text

Constants (Lua and AngelScript):

* `render.FONT_18`, `render.FONT_20`, `render.FONT_24`, `render.FONT_28`
* `render.EFFECT_NONE`, `render.EFFECT_OUTLINE`, `render.EFFECT_SHADOW`, `render.EFFECT_GLOW`

| Function                    | Signature |
|-----------------------------|-----------|
| `draw_text`                 | `(pos, color, text, font?, effect?, effect_color?)` |
| `get_text_size`             | `(text, font?) -> { w, h }` |
| `get_char_advance`          | `(codepoint, font?) -> float` |
| `load_font_from_memory`     | `(ttf_bytes, pixel_size) -> font_id` |
| `load_font_from_file`       | `(path, pixel_size) -> font_id` |

## Bitmaps

| Function          | Signature |
|-------------------|-----------|
| `create_bitmap`   | `(rgba8_bytes, width, height) -> id` |
| `destroy_bitmap`  | `(id)` |
| `draw_bitmap`     | `(id, min, max, tint?, rounding?)` |

RGBA is row-major, top-left origin, 4 bytes per pixel.

## Clipping

Rectangular scissor stack.

* `render.clip_push(min, max)` — subsequent draws are clipped to the rect.
* `render.clip_pop()` — restore the previous clip level.

## Example — a centered animated panel

{% tabs %}
{% tab title="Lua" %}
```lua
function on_frame()
    local vp = render.get_view()
    local w, h = 400, 220
    local mn = vector2.new((vp.x - w) * 0.5, (vp.y - h) * 0.5)
    local mx = vector2.new(mn.x + w, mn.y + h)

    render.draw_rect_filled(mn, mx, color_t.new(15, 15, 20, 220), 8.0)
    render.draw_rect       (mn, mx, color_t.new(80, 200, 255, 220), 1.0, 8.0)

    local pulse = 0.5 + 0.5 * math.sin(os.clock() * 4)
    local accent = color_t.new(80, 200, 255, math.floor(255 * pulse))
    render.draw_rect_filled(mn, vector2.new(mx.x, mn.y + 3), accent, 8.0)

    render.draw_text(vector2.new(mn.x + 16, mn.y + 20),
                     color_t.new(255, 255, 255),
                     "paris",
                     render.FONT_28, render.EFFECT_SHADOW)
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
int on_frame()
{
    vector2 vp = render::get_view();
    float w = 400, h = 220;
    vector2 mn((vp.x - w) * 0.5f, (vp.y - h) * 0.5f);
    vector2 mx(mn.x + w, mn.y + h);

    render::draw_rect_filled(mn, mx, color_t(15, 15, 20, 220), 8.0f);
    render::draw_rect       (mn, mx, color_t(80, 200, 255, 220), 1.0f, 8.0f);

    render::draw_text(vector2(mn.x + 16, mn.y + 20),
                      color_t(255, 255, 255),
                      "paris", 28, 2);
    return 1;
}
```
{% endtab %}
{% endtabs %}
