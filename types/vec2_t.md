---
description: A 2D vector of two floats (x, y).
---

# vec2\_t

`vec2_t` represents a point or direction in 2D space. It is used throughout the
API for screen coordinates, menu positions and 2D math.

## Fields

| Field | Type    | Description        |
| ----- | ------- | ------------------ |
| `x`   | `float` | Horizontal axis.   |
| `y`   | `float` | Vertical axis.     |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local a = vec2_t.new()        -- (0, 0)
local b = vec2_t.new(10, 20)  -- (10, 20)
local c = vec2_t.new(b)       -- copy of b
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
vec2_t a;            // (0, 0)
vec2_t b(10, 20);    // (10, 20)
vec2_t c = b;        // copy of b
```
{% endtab %}
{% endtabs %}

## Methods

### length

Returns the Euclidean length (magnitude) of the vector.

{% tabs %}
{% tab title="Lua" %}
```lua
local len = vec2_t.new(3, 4):length()  -- 5.0
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
float len = vec2_t(3, 4).length();  // 5.0
```
{% endtab %}
{% endtabs %}

### length\_sqr

Returns the squared length. Cheaper than `length()` — prefer it when you only
need to compare distances.

### distance

Returns the distance between two vectors.

{% tabs %}
{% tab title="Lua" %}
```lua
local d = vec2_t.new(0, 0):distance(vec2_t.new(3, 4))  -- 5.0
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
float d = vec2_t(0, 0).distance(vec2_t(3, 4));  // 5.0
```
{% endtab %}
{% endtabs %}

### dot

Returns the dot product with another vector.

### normalized

Returns a unit-length copy pointing in the same direction.

## Operators

Both languages overload the arithmetic operators component-wise.

{% tabs %}
{% tab title="Lua" %}
```lua
local a = vec2_t.new(1, 2)
local b = vec2_t.new(3, 4)

local sum  = a + b   -- (4, 6)
local diff = b - a   -- (2, 2)
local scl  = a * 2   -- (2, 4)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
vec2_t a(1, 2);
vec2_t b(3, 4);

vec2_t sum  = a + b;  // (4, 6)
vec2_t diff = b - a;  // (2, 2)
vec2_t scl  = a * 2;  // (2, 4)
```
{% endtab %}
{% endtabs %}

## Method reference

| Method                | Returns  | Description                          |
| --------------------- | -------- | ------------------------------------ |
| `length()`            | `float`  | Magnitude of the vector.             |
| `length_sqr()`        | `float`  | Squared magnitude (cheaper).         |
| `distance(other)`     | `float`  | Distance to `other`.                 |
| `dot(other)`          | `float`  | Dot product with `other`.            |
| `normalized()`        | `vec2_t` | Unit-length copy.                    |
| `to_string()`         | `string` | `"(x, y)"` for debugging.            |
