---
description: A 3D vector of three floats (x, y, z).
---

# vec3\_t

`vec3_t` represents a point or direction in 3D world space — entity origins,
eye positions, angles and so on.

## Fields

| Field | Type    | Description     |
| ----- | ------- | --------------- |
| `x`   | `float` | X axis.         |
| `y`   | `float` | Y axis.         |
| `z`   | `float` | Z axis (up).    |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local a = vec3_t.new()            -- (0, 0, 0)
local b = vec3_t.new(1, 2, 3)     -- (1, 2, 3)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
vec3_t a;             // (0, 0, 0)
vec3_t b(1, 2, 3);    // (1, 2, 3)
```
{% endtab %}
{% endtabs %}

## Methods

### length / length\_sqr

Magnitude of the vector, and its cheaper squared variant.

### distance

Distance between two points in world space.

{% tabs %}
{% tab title="Lua" %}
```lua
local d = local_origin:distance(enemy_origin)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
float d = local_origin.distance(enemy_origin);
```
{% endtab %}
{% endtabs %}

### dot / cross

Dot product (`float`) and cross product (`vec3_t`) with another vector.

### normalized

Returns a unit-length copy.

### to\_screen

Projects a world position to a 2D screen position. Returns `nil` / a false
success flag when the point is behind the camera.

{% tabs %}
{% tab title="Lua" %}
```lua
local screen = enemy_origin:to_screen()
if screen then
    render.text(screen, color_t.new(255, 0, 0), "enemy")
end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
vec2_t screen;
if (enemy_origin.to_screen(screen))
    render::text(screen, color_t(255, 0, 0), "enemy");
```
{% endtab %}
{% endtabs %}

## Method reference

| Method              | Returns  | Description                             |
| ------------------- | -------- | --------------------------------------- |
| `length()`          | `float`  | Magnitude.                              |
| `length_sqr()`      | `float`  | Squared magnitude.                      |
| `distance(other)`   | `float`  | Distance to `other`.                    |
| `dot(other)`        | `float`  | Dot product.                            |
| `cross(other)`      | `vec3_t` | Cross product.                          |
| `normalized()`      | `vec3_t` | Unit-length copy.                       |
| `to_screen()`       | `vec2_t` | World → screen projection.              |
