# mathx

Constants, scalar helpers, and matrix factories on top of the built-in
[types](../types/vector2.md).

## Constants

| Name             | Value      |
|------------------|------------|
| `mathx.M_PI`     | `π`        |
| `mathx.M_TAU`    | `2π`       |
| `mathx.DEG2RAD`  | `π / 180`  |
| `mathx.RAD2DEG`  | `180 / π`  |

## Scalar helpers

| Function                                          | Purpose |
|---------------------------------------------------|---------|
| `mathx.clamp(v, min, max)`                        | Clamp `v` to `[min, max]`. |
| `mathx.saturate(v)`                               | Shortcut for `clamp(v, 0, 1)`. |
| `mathx.lerp(a, b, t)`                             | Linear interpolation. |
| `mathx.smoothstep(edge0, edge1, x)`               | Hermite smoothstep. |
| `mathx.remap(x, in_min, in_max, out_min, out_max)`| Map a value from one range to another. |
| `mathx.wrap(x, min, max)`                         | Wrap `x` into `[min, max)`. |
| `mathx.inverse_lerp(a, b, v)`                     | Inverse of `lerp` — returns the `t` such that `lerp(a, b, t) = v`. |

## Matrix factories — `mat4`

| Function              | Returns    | Purpose |
|-----------------------|------------|---------|
| `mat4.identity()`     | `matrix4x4`| Identity. |
| `mat4.translation(v)` | `matrix4x4`| Translation by vector3 `v`. |
| `mat4.scaling(v)`     | `matrix4x4`| Scale by vector3 `v`. |
| `mat4.rotation(q)`    | `matrix4x4`| Rotation from a quaternion. |

## Example — pulse an alpha value

```lua
function on_frame()
    local t = os.clock() * 3.0
    local pulse = mathx.smoothstep(0.2, 0.8,
                                    math.abs(mathx.wrap(t, -1.0, 1.0)))
    local col = color_t.new(255, 255, 255, math.floor(255 * pulse))
    render.draw_text(vector2.new(20, 20), col, "pulsing", render.FONT_20)
    return 1
end
```
