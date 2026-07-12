# vector2

A 2D point or direction — screen coordinates, menu positions, 2D math. Also
available under the alias `vec2_t` for backward compatibility.

## Fields

| Name | Type  | Purpose            |
|------|-------|--------------------|
| `x`  | float | horizontal         |
| `y`  | float | vertical           |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local a = vector2.new()          -- (0, 0)
local b = vector2.new(10, 20)    -- (10, 20)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
vector2 a;                       // (0, 0)
vector2 b(10, 20);               // (10, 20)
```
{% endtab %}
{% endtabs %}

## Methods

| Method            | Returns  | Purpose |
|-------------------|----------|---------|
| `length()`        | `float`  | Euclidean magnitude |
| `length_sqr()`    | `float`  | Squared magnitude (cheaper for comparisons) |
| `distance(other)` | `float`  | Distance between two points |
| `dot(other)`      | `float`  | Dot product |
| `normalized()`    | `vector2`| Unit-length copy |
| `to_string()`     | `string` | Debug representation |

## Operators

Both languages support `+`, `-`, and `* scalar`.

{% tabs %}
{% tab title="Lua" %}
```lua
local pos    = vector2.new(100, 200)
local delta  = vector2.new(-5, 3)
local moved  = pos + delta
local scaled = moved * 0.5
print(moved:to_string(), scaled:to_string())
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
vector2 pos(100, 200);
vector2 delta(-5, 3);
vector2 moved  = pos + delta;
vector2 scaled = moved * 0.5f;
engine::log(moved.to_string());
```
{% endtab %}
{% endtabs %}
