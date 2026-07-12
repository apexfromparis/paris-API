# Subtabs and panels

## Create a subtab

Subtabs live under a fixed top-level tab (index `0` to `4`). Pick the one that
fits your script's category — the exact naming of each tab depends on the paris
build; treat the index as a lane, not a label.

{% tabs %}
{% tab title="Lua" %}
```lua
local sub = ui.create_subtab(0, "My Script")
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Subtab@ sub = ui::create_subtab(0, "My Script");
```
{% endtab %}
{% endtabs %}

## Add panels

Panels group related elements. `is_small` renders the panel at half width so
you can pack more of them side by side.

{% tabs %}
{% tab title="Lua" %}
```lua
local main_panel   = sub:add_panel("Main",   false)
local small_panel  = sub:add_panel("Extras", true)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
Panel@ main_panel  = sub.add_panel("Main",   false);
Panel@ small_panel = sub.add_panel("Extras", true);
```
{% endtab %}
{% endtabs %}

## `set_active`

Toggle the visibility of a whole subtab. Useful when a script wants to hide
irrelevant options depending on the current game mode.

{% tabs %}
{% tab title="Lua" %}
```lua
sub:set_active(false)  -- hide entire subtab
sub:set_active(true)   -- restore
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
sub.set_active(false);
sub.set_active(true);
```
{% endtab %}
{% endtabs %}

`Element.set_active(bool)` does the same at the element granularity.

## Cross-script lookup

If another script owns an element you want to read, use
[`ui.find_element`](README.md). Look-ups are name-based and return `nil` when
the target doesn't exist yet — safe to call at any time.
