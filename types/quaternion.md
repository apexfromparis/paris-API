# quaternion

Rotation represented as a unit quaternion `w + xi + yj + zk`. Field order is
`x, y, z, w` so the memory layout matches most game engines (Source, Unreal,
Unity).

## Fields

| Name | Type  |
|------|-------|
| `x`  | float |
| `y`  | float |
| `z`  | float |
| `w`  | float |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local q0 = quaternion.new()                   -- identity (0, 0, 0, 1)
local q1 = quaternion.new(0, 0, 0.7071, 0.7071)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
quaternion q0;                                // identity
quaternion q1(0, 0, 0.7071f, 0.7071f);
```
{% endtab %}
{% endtabs %}

## Statics

| Function                                | Returns      | Purpose |
|-----------------------------------------|--------------|---------|
| `quaternion.identity()`                 | `quaternion` | Identity rotation. |
| `quaternion.from_euler(rx, ry, rz)`     | `quaternion` | Euler XYZ intrinsic (radians). |

## Methods

| Method            | Returns      | Purpose |
|-------------------|--------------|---------|
| `rotate(v)`       | `vector3`    | Rotate a vector by this quaternion. |
| `inverse()`       | `quaternion` | Inverse rotation. |
| `to_euler()`      | `vector3`    | Convert back to Euler XYZ (radians). |
| `operator*`       | `quaternion` | Compose two rotations. |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
-- rotate a forward vector by yaw + pitch
local rot   = quaternion.from_euler(pitch, 0, yaw)
local aimed = rot:rotate(vector3.new(1, 0, 0))
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
quaternion rot   = quaternion::from_euler(pitch, 0, yaw);
vector3    aimed = rot.rotate(vector3(1, 0, 0));
```
{% endtab %}
{% endtabs %}
