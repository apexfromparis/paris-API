# matrix4x4

Row-major 4x4 matrix. `m[row][col]` — that matches DirectX and streams into
HLSL constant buffers without a transpose.

## Access

{% tabs %}
{% tab title="Lua" %}
```lua
local m = matrix4x4.new()  -- identity
print(m:at(0, 0))          -- 1
m:at(0, 3, 5.0)            -- set translation X
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
matrix4x4 m;
engine::log("" + m.at(0, 0));   // "1"
// AngelScript exposes at() as read-only in this port; use mat4::translation to build
```
{% endtab %}
{% endtabs %}

## Statics — the `mat4` namespace

Prefer these factories over hand-filling the matrix.

| Function              | Returns    | Purpose |
|-----------------------|------------|---------|
| `mat4.identity()`     | `matrix4x4`| Identity. |
| `mat4.translation(v)` | `matrix4x4`| Translation by vector3 `v`. |
| `mat4.scaling(v)`     | `matrix4x4`| Non-uniform scale by vector3 `v`. |
| `mat4.rotation(q)`    | `matrix4x4`| Rotation from a quaternion. |

## Methods

| Method            | Returns    | Purpose |
|-------------------|------------|---------|
| `transform(v)`    | `vector3`  | Multiply the matrix by `v` (interpreted as `(v.x, v.y, v.z, 1)`). |
| `operator*`       | `matrix4x4`| Matrix product. |

## Example — compose a transform

{% tabs %}
{% tab title="Lua" %}
```lua
local T = mat4.translation(vector3.new(0, 0, 10))
local R = mat4.rotation(quaternion.from_euler(0, 0, math.pi * 0.25))
local S = mat4.scaling(vector3.new(1, 1, 1))
local M = T * R * S
local p = M:transform(vector3.new(1, 0, 0))
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
matrix4x4 T = mat4::translation(vector3(0, 0, 10));
matrix4x4 R = mat4::rotation(quaternion::from_euler(0, 0, 0.7854f));
matrix4x4 S = mat4::scaling(vector3(1, 1, 1));
matrix4x4 M = T * R * S;
vector3   p = M.transform(vector3(1, 0, 0));
```
{% endtab %}
{% endtabs %}
