# UI API

paris' menu is a three-level tree:

```
Tab (0..4, fixed)
└── Subtab   (created by scripts via ui.create_subtab)
    └── Panel   (created by subtab:add_panel)
        └── Element  (checkbox, slider, input, …)
```

Each script picks a top-level tab index (`0` through `4`), creates one or more
subtabs there, adds panels, and fills each panel with interactive elements.

## The lifecycle rule

Elements are **created once**, at load time. Store the handle you get back
and read its value each frame from `on_frame`.

{% hint style="warning" %}
Calling `panel:add_checkbox` (or any other `add_*`) from `on_frame` will spawn
a new element every frame and freeze the menu.
{% endhint %}

## Minimum viable UI

{% tabs %}
{% tab title="Lua" %}
```lua
local sub, panel, enabled

function main()
    sub     = ui.create_subtab(0, "My Feature")
    panel   = sub:add_panel("Settings", false)
    enabled = panel:add_checkbox("Enable", false)
    return 1
end

function on_frame()
    if enabled:get() then
        -- your feature is on
    end
    return 1
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
    @sub     = ui::create_subtab(0, "My Feature");
    @panel   = sub.add_panel("Settings", false);
    @enabled = panel.add_checkbox("Enable", false);
    return 1;
}

int on_frame()
{
    if (enabled.get()) { /* your feature is on */ }
    return 1;
}
```
{% endtab %}
{% endtabs %}

## Element types

| Kind                                | Returns                | Purpose |
|-------------------------------------|------------------------|---------|
| [checkbox](checkbox.md)             | `bool`                 | Toggle. Can host child keybinds and color pickers. |
| [slider](slider.md)                 | `int` or `double`      | Numeric range. |
| [input](input.md)                   | `string`               | Free-text field. |
| [multi\_select](multi_select.md)    | `{bool...}`            | Multi-choice checklist. |
| [single\_select](single_select.md)  | `int`                  | Dropdown, returns the selected index. |
| [keybind](keybind.md)               | `bool` (`is_pressed`)  | Hotkey with mode (`on` / `toggle` / `single` / `always_on`). Attached to a checkbox. |
| [color\_picker](color_picker.md)    | `color_t`              | RGBA color. Attached to a checkbox. |
| [button](button.md)                 | —                      | Runs a callback when clicked. |
| [list](list.md)                     | active index / rows    | Scrollable list of `{ name, info }` rows. |

## Handle methods

Every element handle exposes at least `set_active(bool)`. Most have `get()`
and `set(...)` too. The details are on each element's page.

## Other utilities

* [Subtabs and panels](subtabs-and-panels.md) — creation, options, `set_active`.
* [Configs](configs.md) — save / restore every element's value with two calls.
* `ui.is_active()` — returns `true` while the menu is on-screen. Useful for
  skipping expensive draws when nobody's looking.
* `ui.find_element(tab_idx, subtab, panel, element, type)` — cross-script
  handle lookup by name. Type is one of `"checkbox"`, `"slider_int"`,
  `"slider_double"`, `"input"`, `"multi_select"`, `"single_select"`,
  `"keybind"`, `"color_picker"`, `"button"`, `"list"`.
