# keybind

A hotkey. Always attached to a parent [checkbox](checkbox.md). paris supports
several activation modes so the same bind can be a toggle, a hold, or a
one-shot.

{% hint style="warning" %}
Create the parent checkbox **first**, then immediately add the keybind. A
checkbox can host at most two keybinds and two color pickers combined; add
them in the order you want them to appear.
{% endhint %}

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_keybind(parent_checkbox, name, vk_code, mode,
                  draw_title?, find_protect?)
```
{% endtab %}
{% tab title="AngelScript" %}
Lua only for now.
{% endtab %}
{% endtabs %}

| Parameter    | Type      | Purpose |
|--------------|-----------|---------|
| `parent`     | Checkbox  | The checkbox this keybind gates. |
| `name`       | string    | Label. |
| `vk_code`    | int       | Windows virtual-key code (`0x2D` for Insert, `0x70` for F1, `0x20` for Space, …). |
| `mode`       | string    | See table below. |

## Modes

| Mode         | Behaviour |
|--------------|-----------|
| `"off"`      | Never fires. Effectively disables the bind. |
| `"on"`       | Fires while the key is held. |
| `"single"`   | Fires once per press. |
| `"toggle"`   | Toggles state on each press. |
| `"always_on"`| Always fires — the key is irrelevant. |

## Methods

| Method              | Signature |
|---------------------|-----------|
| `is_pressed()`      | `bool` — evaluated according to the current mode. |
| `set(vk, mode)`     | Rebind at runtime. Mode is an int (`0`=off, `1`=on, `2`=single, `3`=toggle, `4`=always_on). |
| `set_active(bool)`  | show/hide |

## Example

```lua
local aim_enable = panel:add_checkbox("Aimbot", false)
local aim_key    = panel:add_keybind(aim_enable, "Aim key", 0x02, "on") -- RMB

function on_frame()
    if aim_key:is_pressed() then
        run_aimbot()
    end
    return 1
end
```
