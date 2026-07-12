# vector3

A 3D point or direction — entity origins, eye positions, view angles.
Also available under the alias `vec3_t`.

## Fields

| Name | Type  | Purpose        |
|------|-------|----------------|
| `x`  | float | X axis         |
| `y`  | float | Y axis         |
| `z`  | float | Z axis (up)    |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local a = vector3.new()
local b = vector3.new(1, 2, 3)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
vector3 a;
vector3 b(1, 2, 3);
```
{% endtab %}
{% endtabs %}

## Methods

| Method              | Returns   | Purpose |
|---------------------|-----------|---------|
| `length()`          | `float`   | Magnitude |
| `length_sqr()`      | `float`   | Squared magnitude |
| `distance(other)`   | `float`   | Distance between two points |
| `dot(other)`        | `float`   | Dot product |
| `cross(other)`      | `vector3` | Cross product |
| `normalized()`      | `vector3` | Unit-length copy |
| `to_screen()`       | see below | World-to-screen projection |
| `to_string()`       | `string`  | Debug representation |

## `to_screen`

Projects a world-space point through the camera and returns the resulting
2D coordinate. Returns nothing when the point is behind the camera.

{% tabs %}
{% tab title="Lua" %}
```lua
local screen = enemy_origin:to_screen()
if screen then
    render.draw_text(screen, color_t.new(255, 0, 0), "enemy")
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
vector2 screen;
if (enemy_origin.to_screen(screen))
    render::draw_text(screen, color_t(255, 0, 0), "enemy");
```
{% endtab %}
{% endtabs %}

## Operators

Both languages support `+`, `-`, and `* scalar`.
