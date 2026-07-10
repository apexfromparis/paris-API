---
description: Add your own controls to the paris menu.
---

# GUI API

The GUI API lets your script add controls — checkboxes, sliders, combos,
buttons and color pickers — to the paris menu. Your controls appear under a
section named after your script.

## Pattern

Every control follows the same three-step pattern:

1. **Create** the control once, at load time. Creation returns a **handle**.
2. **Store** the handle in a variable.
3. **Read** its value each frame with `:get()` (or write it with `:set()`).

{% tabs %}
{% tab title="Lua" %}
```lua
-- 1 + 2: create once, store the handle
local enabled   = gui.checkbox("Enable")
local thickness = gui.slider("Thickness", 1, 1, 10)

local function on_paint()
    -- 3: read every frame
    if enabled:get() then
        draw_box(thickness:get())
    end
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
// 1 + 2: create once, store the handle
Checkbox@ enabled   = gui::checkbox("Enable");
Slider@   thickness = gui::slider("Thickness", 1, 1, 10);

void on_paint()
{
    // 3: read every frame
    if (enabled.get())
        draw_box(thickness.get());
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}

## Controls

| Control                              | Value type |
| ------------------------------------ | ---------- |
| [checkbox](checkbox.md)              | `bool`     |
| [slider](slider.md)                  | `int` / `float` |
| [combo](combo.md)                    | `int` (selected index) |
| [button](button.md)                  | — (callback) |
| [color\_picker](color_picker.md)     | `color_t`  |

{% hint style="danger" %}
Never create controls inside a callback. Doing so adds a new control every
frame and will freeze the menu.
{% endhint %}
