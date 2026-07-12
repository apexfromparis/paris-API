# list

Scrollable list of `{ name, info }` rows. Supports live append, per-row
highlighting, and selection.

## Signature

{% tabs %}
{% tab title="Lua" %}
```lua
panel:add_list(name, entries,
               draw_title?, find_protect?)
```
Where `entries` is a table of `{ name = "...", info = "..." }`.
{% endtab %}
{% tab title="AngelScript" %}
Lua only for now.
{% endtab %}
{% endtabs %}

## Methods

| Method                     | Signature |
|----------------------------|-----------|
| `append(name, info)`       | Add a row at the end. |
| `append_after(name, info, index)` | Insert a row after `index`. |
| `get()`                    | `int` — currently-active index (`-1` if none). |
| `get_all()`                | Table of every row. |
| `get_count()`              | Total rows. |
| `clear()`                  | Drop everything. |
| `highlight(index)`         | Mark a row as highlighted (renderer styles it differently). |
| `remove_highlight(index)`  | Undo `highlight`. |
| `remove(index)`            | Drop a specific row. |
| `set_active_index(index)`  | Select a row programmatically. |
| `set_active(bool)`         | show/hide the whole element. |

## Example — live-updating player list

```lua
local players = panel:add_list("Players", {})

function on_frame()
    players:clear()
    for _, p in ipairs(get_all_players()) do
        players:append(p.name, string.format("HP %d", p.health))
        if p.hostile then players:highlight(players:get_count() - 1) end
    end
    return 1
end
```
