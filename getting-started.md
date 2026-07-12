# Getting started

A paris script is a single source file loaded from your scripts directory.
The engine supports two languages — **Lua** and **AngelScript** — with the
same API surface.

## Your first script

Every script defines at least `main()` and returns a positive integer to keep
running. Add `on_frame()` for per-frame work and `on_unload()` for cleanup.

{% tabs %}
{% tab title="Lua" %}
```lua
local sub, panel, enabled

function main()
    sub     = ui.create_subtab(0, "My Script")
    panel   = sub:add_panel("Settings", false)
    enabled = panel:add_checkbox("Enable overlay", false)
    engine.log("hello from " .. engine.get_user_name())
    return 1
end

function on_frame()
    if enabled:get() then
        render.draw_text(vector2.new(20, 20),
                         color_t.new(255, 255, 255),
                         "overlay active",
                         render.FONT_20, render.EFFECT_SHADOW)
    end
    return 1
end

function on_unload()
    engine.log("my_script unloading")
end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Subtab@   sub;
Panel@    panel;
Checkbox@ enabled;

int main()
{
    @sub     = ui::create_subtab(0, "My Script");
    @panel   = sub.add_panel("Settings", false);
    @enabled = panel.add_checkbox("Enable overlay", false);
    engine::log("hello from " + engine::get_user_name());
    return 1;
}

int on_frame()
{
    if (enabled.get())
        render::draw_text(vector2(20, 20),
                          color_t(255, 255, 255),
                          "overlay active",
                          20, 2);
    return 1;
}

void on_unload()
{
    engine::log("my_script unloading");
}
```
{% endtab %}
{% endtabs %}

## Loading a script

Drop your `.lua` or `.as` file into paris' `scripts/` directory. paris loads
every script found there at startup and rebuilds them automatically when they
change on disk.

## What happens when the script runs

| Step | What paris does |
|------|-----------------|
| 1. Load | Reads the file, opens a fresh sandbox, calls `main()`. |
| 2. Persist check | If `main()` returned `> 0` and the script defines `on_frame()`, keeps it alive. |
| 3. Per frame | Calls `on_frame()` every render tick. Returning `<= 0` unloads the script. |
| 4. Unload | Calls `on_unload()`, drops the UI, drops the callbacks. |

## Rules

{% hint style="warning" %}
Create your UI elements **once** in `main()`. Never call `panel:add_checkbox`
from `on_frame` — you'd spawn a new control every frame and freeze the menu.
{% endhint %}

{% hint style="warning" %}
`on_frame` runs on the render thread — keep it cheap. Heavy work (HTTP
requests, big JSON, disk I/O) belongs in a button callback or a coroutine.
{% endhint %}

## Next up

* [Life cycle](life-cycle.md) — the full contract for `main`, `on_frame`, `on_unload`
* [UI API](ui/README.md) — every element kind, hierarchy rules, config save/load
* [Modules](modules/README.md) — render, input, fs, mathx, util, json, net, engine, callbacks
