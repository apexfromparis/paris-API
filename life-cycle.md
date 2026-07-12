# Life cycle

Every paris script goes through the same three stages: load, run, unload. Only
`main` is mandatory. The other two are optional but almost always useful.

## Required: `main`

Runs exactly once when the script is loaded. Its return value decides whether
the script keeps running.

| Return value | + `on_frame` defined? | Result |
|--------------|----------------------|--------|
| `> 0`        | yes                  | Script persists, `on_frame` starts firing. |
| `> 0`        | no                   | Script is dropped immediately after `main`. |
| `<= 0`       | any                  | Script is dropped immediately after `main`. |

If `main` throws or errors out, the script is dropped and the error is written
to the debug console.

## Optional: `on_frame`

Runs on every render tick. Return `> 0` to keep going, `<= 0` to unload the
script. If you don't return anything, the script keeps running (treated as
`1`).

{% hint style="warning" %}
`on_frame` is the hot path. Avoid:
* Building UI elements
* Blocking network / disk calls
* Allocating large temporary tables

Keep the frame budget under a millisecond so paris stays smooth.
{% endhint %}

## Optional: `on_unload`

Runs once, right before the script is dropped. Use this to close network
handles, save state, or emit a final log line.

## Full skeleton

{% tabs %}
{% tab title="Lua" %}
```lua
local subtab, panel
local enabled

function main()
    subtab  = ui.create_subtab(0, "My Script")
    panel   = subtab:add_panel("Settings", false)
    enabled = panel:add_checkbox("Enable", false)
    engine.log("script loaded")
    return 1
end

function on_frame()
    if not enabled:get() then return 1 end
    -- render / logic
    return 1
end

function on_unload()
    engine.log("script unloading")
end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Subtab@   subtab;
Panel@    panel;
Checkbox@ enabled;

int main()
{
    @subtab  = ui::create_subtab(0, "My Script");
    @panel   = subtab.add_panel("Settings", false);
    @enabled = panel.add_checkbox("Enable", false);
    engine::log("script loaded");
    return 1;
}

int on_frame()
{
    if (!enabled.get()) return 1;
    // render / logic
    return 1;
}

void on_unload()
{
    engine::log("script unloading");
}
```
{% endtab %}
{% endtabs %}

## Hot reload

paris watches your scripts directory. When a file changes on disk:

1. The old instance runs `on_unload`
2. Its UI subtree and callbacks are dropped
3. The new source is loaded and `main` runs

You don't have to restart paris — just save the file.
