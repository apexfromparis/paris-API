# button

Clickable button. Runs the supplied callback once per click.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_button(name, callback)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Button@ panel.add_button(const string &in name, callback_t @fn);
```
Where `callback_t` is a `void()` funcdef.
{% endtab %}
{% endtabs %}

| Parameter | Purpose |
|-----------|---------|
| `name`    | Button label. |
| `callback`| Function that runs on click. No arguments, no return value. |

## Methods

Buttons have no `get()` — they're event sources.

| Method              | Signature |
|---------------------|-----------|
| `set_active(bool)`  | show/hide |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_button("Reset config", function()
    enabled:set(false)
    thickness:set(1.5)
    engine.log("config reset")
end)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
void on_reset()
{
    enabled.set(false);
    thickness.set(1.5);
    engine::log("config reset");
}

int main()
{
    panel.add_button("Reset config", @on_reset);
    return 1;
}
```
{% endtab %}
{% endtabs %}
