---
description: A clickable button that runs a callback.
---

# button

Adds a button. Instead of holding a value, it runs a function when clicked.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
gui.button(label: string, callback: function) -> Button
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Button@ gui::button(const string &in label, callback_t @fn)
```
{% endtab %}
{% endtabs %}

## Parameters

| Parameter  | Type       | Description                        |
| ---------- | ---------- | ---------------------------------- |
| `label`    | `string`   | Text shown on the button.          |
| `callback` | `function` | Called once each time it's clicked.|

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
gui.button("Reset config", function()
    config.reset()
    print("config reset")
end)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
void on_reset()
{
    config::reset();
    print("config reset");
}

void main()
{
    gui::button("Reset config", @on_reset);
}
```
{% endtab %}
{% endtabs %}
