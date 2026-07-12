# slider

Two flavours: `slider_int` for whole numbers, `slider_double` for decimals.

## Signatures

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_slider_int    (name, postfix, value, min, max, step,
                         draw_title?, find_protect?)
panel:add_slider_double (name, postfix, value, min, max, step,
                         draw_title?, find_protect?)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
SliderInt@    panel.add_slider_int   (const string &in name,
                                      const string &in postfix,
                                      int value, int min, int max, int step);
SliderDouble@ panel.add_slider_double(const string &in name,
                                      const string &in postfix,
                                      double value, double min, double max, double step);
```
{% endtab %}
{% endtabs %}

| Parameter | Purpose |
|-----------|---------|
| `name`    | Text above the slider. |
| `postfix` | Suffix appended to the value (e.g. `"px"`, `"°"`). |
| `value`   | Initial value. |
| `min` / `max` | Range endpoints. |
| `step`    | Minimum increment. |

## Methods

Both slider kinds expose `get()`, `set(value)`, and `set_active(bool)`. Types
differ: int for `slider_int`, double for `slider_double`.

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local fov       = panel:add_slider_double("FOV",        "°",  90.0, 40.0, 120.0, 0.5)
local thickness = panel:add_slider_int   ("Box border", "px",  2,   1,    5,    1)

function on_frame()
    render.draw_rect(vector2.new(100, 100),
                     vector2.new(200, 260),
                     color_t.new(0, 255, 120),
                     thickness:get())
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
SliderDouble@ fov;
SliderInt@    thickness;

int main()
{
    @fov       = panel.add_slider_double("FOV",        "\xC2\xB0", 90.0, 40.0, 120.0, 0.5);
    @thickness = panel.add_slider_int   ("Box border", "px",       2,   1,    5,    1);
    return 1;
}
```
{% endtab %}
{% endtabs %}
