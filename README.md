---
description: Official scripting API for paris.
---

# Introduction

Welcome to the **paris scripting API**. paris embeds a full-featured scripting
engine — you can write extensions in **Lua** or **AngelScript** using the same
API surface. This documentation covers every module: UI, rendering, input,
filesystem, math, networking, encoding, JSON, and lifecycle events.

Every page shows both languages side by side. Pick the tab for the language you
write in:

{% tabs %}
{% tab title="Lua" %}
```lua
function main()
    local sub    = ui.create_subtab(0, "My Script")
    local panel  = sub:add_panel("Settings", false)
    local hello  = panel:add_checkbox("Say hello", true)
    return 1
end

function on_frame()
    if hello:get() then
        render.draw_text(vector2.new(20, 20),
                         color_t.new(255, 255, 255),
                         "hello from paris")
    end
    return 1
end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Subtab@   sub;
Panel@    panel;
Checkbox@ hello;

int main()
{
    @sub   = ui::create_subtab(0, "My Script");
    @panel = sub.add_panel("Settings", false);
    @hello = panel.add_checkbox("Say hello", true);
    return 1;
}

int on_frame()
{
    if (hello.get())
        render::draw_text(vector2(20, 20),
                          color_t(255, 255, 255),
                          "hello from paris");
    return 1;
}
```
{% endtab %}
{% endtabs %}

## Sections

* [Getting started](getting-started.md) — install a script, understand the lifecycle
* [Life cycle](life-cycle.md) — `main`, `on_frame`, `on_unload`
* [Types](types/vector2.md) — vectors, colors, quaternions, matrices
* [UI API](ui/README.md) — subtabs, panels, and every element kind
* [Modules](modules/README.md) — render, input, fs, mathx, util, json, net, engine, callbacks

{% hint style="info" %}
Scripts are published in the **Script Market** on the forum by certified
developers. Ask an admin for the `developper` role to publish yours.
{% endhint %}

{% hint style="warning" %}
Create your UI elements **once** in `main()`. Storing handles and calling
`:get()` from `on_frame()` is the correct pattern — building UI every frame
will freeze the menu.
{% endhint %}
