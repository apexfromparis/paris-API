---
description: A boolean on/off control.
---

# checkbox

Adds a checkbox to the menu. Returns a handle whose value is a `bool`.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
gui.checkbox(label: string) -> Checkbox
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Checkbox@ gui::checkbox(const string &in label)
```
{% endtab %}
{% endtabs %}

## Parameters

| Parameter | Type     | Description                        |
| --------- | -------- | ---------------------------------- |
| `label`   | `string` | Text shown next to the checkbox.   |

## Handle methods

| Method        | Returns | Description                    |
| ------------- | ------- | ----------------------------- |
| `get()`       | `bool`  | Current state.                |
| `set(value)`  | —       | Force the state.              |

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
local enabled = gui.checkbox("Enable ESP")

local function on_paint()
    if enabled:get() then
        -- draw ESP
    end
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Checkbox@ enabled = gui::checkbox("Enable ESP");

void on_paint()
{
    if (enabled.get())
    {
        // draw ESP
    }
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}
