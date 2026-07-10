---
description: An RGBA color, 0-255 per channel.
---

# color\_t

`color_t` holds a color as four 8-bit channels: red, green, blue and alpha.
It is accepted by every rendering and GUI function that draws something.

## Fields

| Field | Type  | Range   | Description         |
| ----- | ----- | ------- | ------------------- |
| `r`   | `int` | 0–255   | Red channel.        |
| `g`   | `int` | 0–255   | Green channel.      |
| `b`   | `int` | 0–255   | Blue channel.       |
| `a`   | `int` | 0–255   | Alpha (opacity).    |

## Constructors

{% tabs %}
{% tab title="Lua" %}
```lua
local white = color_t.new(255, 255, 255)       -- a defaults to 255
local red   = color_t.new(255, 0, 0, 128)      -- 50% transparent red
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
color_t white(255, 255, 255);       // a defaults to 255
color_t red(255, 0, 0, 128);        // 50% transparent red
```
{% endtab %}
{% endtabs %}

## Methods

### with\_alpha

Returns a copy with a different alpha. Handy for fading effects.

{% tabs %}
{% tab title="Lua" %}
```lua
local base = color_t.new(0, 255, 120)
local faded = base:with_alpha(80)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
color_t base(0, 255, 120);
color_t faded = base.with_alpha(80);
```
{% endtab %}
{% endtabs %}

### lerp

Linearly interpolates toward another color by `t` (0.0–1.0). Useful for
health-based color gradients.

{% tabs %}
{% tab title="Lua" %}
```lua
local red   = color_t.new(255, 0, 0)
local green = color_t.new(0, 255, 0)
local health_color = red:lerp(green, hp / 100)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
color_t red(255, 0, 0);
color_t green(0, 255, 0);
color_t health_color = red.lerp(green, hp / 100.0f);
```
{% endtab %}
{% endtabs %}

## Helpers

| Constructor / method       | Description                                   |
| -------------------------- | --------------------------------------------- |
| `color_t.new(r, g, b[, a])`| From RGBA channels.                           |
| `color_t.from_hsv(h, s, v)`| From hue/saturation/value (for rainbow FX).   |
| `with_alpha(a)`            | Copy with a new alpha.                         |
| `lerp(other, t)`           | Blend toward `other` by `t`.                   |
