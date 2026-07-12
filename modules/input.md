# input

Frame-based input state. paris snapshots the mouse, keyboard, and typed text
once per frame before firing `on_frame`; every function on this page reads
from that snapshot.

{% hint style="info" %}
Call these functions from `on_frame` (or handlers that run during a frame).
Reading input outside a frame boundary returns stale data.
{% endhint %}

## Mouse — `input.mouse`

| Function                         | Returns   | Purpose |
|----------------------------------|-----------|---------|
| `get_position()`                 | `vector2` | Overlay-relative cursor position. |
| `get_position_desktop()`         | `vector2` | Absolute screen coordinates. |
| `get_delta()`                    | `vector2` | Movement since last frame. |
| `wheel_delta()`                  | `float`   | Notches, positive = up. |
| `hover_rect(min, max)`           | `bool`    | Cursor is inside the rect. |
| `is_down(vk_button)`             | `bool`    | Button is held. |
| `is_pressed(vk_button)`          | `bool`    | Button was just pressed this frame. |

Mouse button VK codes: `0x01` LMB, `0x02` RMB, `0x04` MMB, `0x05` MB4, `0x06` MB5.

## Keyboard — `input.keyboard`

| Function                    | Returns         | Purpose |
|-----------------------------|-----------------|---------|
| `is_down(vk)`               | `bool`          | Held. |
| `is_pressed(vk)`            | `bool`          | Just went down this frame. |
| `is_released(vk)`           | `bool`          | Just went up this frame. |
| `is_toggle_on(vk)`          | `bool`          | Caps-lock-style toggle latch. |
| `is_os_down(vk)`            | `bool`          | Raw `GetAsyncKeyState`, bypasses menu focus. |
| `get_pressed_keys()`        | table of ints   | Every currently-held VK. |
| `vk_name(vk)`               | `string`        | Human name for a VK code. |

## Text — `input.text`

* `get_recent() -> string` — UTF-8 buffer of characters typed since the last
  call. Draining — subsequent calls only see new characters.

Not exposed in AngelScript.

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
function on_frame()
    local m = input.mouse.get_position()
    render.draw_text(vector2.new(20, 20),
                     color_t.new(255, 255, 255),
                     string.format("cursor %.0f, %.0f", m.x, m.y),
                     render.FONT_18)

    if input.keyboard.is_pressed(0x70) then  -- F1
        engine.log("F1 pressed")
    end
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
int on_frame()
{
    vector2 m = input::mouse::get_position();
    if (input::keyboard::is_pressed(0x70))
        engine::log("F1 pressed");
    return 1;
}
```
{% endtab %}
{% endtabs %}
