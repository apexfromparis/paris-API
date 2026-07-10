# Getting started

A paris script is a single source file that the loader runs. The same script
model is available in two languages — **Lua** and **AngelScript** — with a
shared API surface.

## Your first script

{% tabs %}
{% tab title="Lua" %}
```lua
-- registered once when the script loads
local enabled = gui.checkbox("Enable ESP")

-- called every frame
local function on_paint()
    if not enabled:get() then return end
    render.text(vec2_t.new(10, 10), color_t.new(255, 255, 255), "ESP on")
end

callbacks.add("paint", on_paint)
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
// registered once when the script loads
Checkbox@ enabled = gui::checkbox("Enable ESP");

// called every frame
void on_paint()
{
    if (!enabled.get()) return;
    render::text(vec2_t(10, 10), color_t(255, 255, 255), "ESP on");
}

void main()
{
    callbacks::add("paint", @on_paint);
}
```
{% endtab %}
{% endtabs %}

## Lifecycle

| Stage      | Lua                        | AngelScript                     |
| ---------- | -------------------------- | ------------------------------- |
| Load       | top-level code runs once   | `void main()` runs once         |
| Per frame  | `paint` callback           | `paint` callback                |
| Unload     | `shutdown` callback        | `shutdown` callback             |

{% hint style="warning" %}
Create your GUI controls **once**, at load time — never inside a per-frame
callback. Store the handle (like `enabled` above) and call `:get()` each frame.
{% endhint %}
