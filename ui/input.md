# input

Free-text field. Great for API keys, custom labels, chat targets.

{% hint style="info" %}
Not to be confused with the [`input` module](../modules/input.md), which is
the keyboard / mouse API. This page is about the text-entry UI element.
{% endhint %}

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_input(name, initial_text,
                draw_title?, find_protect?)
```
{% endtab %}
{% tab title="AngelScript" %}
This element kind is available in Lua only for now.
{% endtab %}
{% endtabs %}

| Parameter      | Type   | Purpose |
|----------------|--------|---------|
| `name`         | string | Label above the input. |
| `initial_text` | string | Starting value. |
| `draw_title`   | bool   | Hide label when `false`. |
| `find_protect` | bool   | Skip in config serialisation. |

## Methods

| Method             | Signature |
|--------------------|-----------|
| `get()`            | `string`  |
| `set(text)`        | `string`  |
| `set_active(bool)` | show/hide |

## Example

```lua
local nickname = panel:add_input("Nickname", "player_1")

panel:add_button("Print my name", function()
    engine.log("hello " .. nickname:get())
end)
```
