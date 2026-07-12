# Configs

paris can serialise every UI element's current value into a plain-text blob
and restore it later. Use it to build config presets, share setups between
users, or auto-persist on shutdown.

## The format

Each line encodes one element:

```
<tab_index>.<subtab>.<panel>.<element> = <value>
```

Values are encoded compactly per element kind: `1` / `0` for booleans,
comma-separated for lists, quoted for strings. Child elements attached to a
checkbox (color pickers, keybinds) use `element/child` in the path.

`find_protect = true` skips an element from the output — use it for volatile
state (e.g. a scratch input field) that shouldn't be part of the preset.

## Serialise

{% tabs %}
{% tab title="Lua" %}
```lua
local blob = ui.construct_config()
fs.create_file("configs/rage.cfg", blob)
```
{% endtab %}
{% tab title="AngelScript" %}
Not currently exposed in AngelScript.
{% endtab %}
{% endtabs %}

## Restore

{% tabs %}
{% tab title="Lua" %}
```lua
local ok, blob = fs.read_file("configs/rage.cfg")
if ok then ui.apply_config(blob) end
```
{% endtab %}
{% tab title="AngelScript" %}
Not currently exposed in AngelScript.
{% endtab %}
{% endtabs %}

## Recipe — auto-save on unload, auto-load on start

```lua
function main()
    -- build UI first, THEN restore state
    build_ui()
    local ok, blob = fs.read_file("configs/autosave.cfg")
    if ok then ui.apply_config(blob) end
    return 1
end

function on_unload()
    fs.create_file("configs/autosave.cfg", ui.construct_config())
end
```
