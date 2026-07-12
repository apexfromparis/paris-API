# callbacks

Event bus. Register handlers for arbitrary event names.

## Registration

Handlers are tagged with the current script; unloading the script drops all
its handlers automatically.

{% tabs %}
{% tab title="Lua" %}
```lua
callbacks.add(event_name, function()
    -- your handler
end)
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
callbacks::add(event_name, @your_handler);
```

Where `your_handler` matches the `callback_t` funcdef.
{% endtab %}
{% endtabs %}

## Built-in events

paris always fires:

| Event         | When |
|---------------|------|
| `on_frame`    | Every render tick, before your script's own `on_frame`. |
| `on_unload`   | Once, when the host shuts down. |

Additional events depend on the paris build. Fire your own from the host
via `paris::callbacks::fire("your_event")`.

## Example

{% tabs %}
{% tab title="Lua" %}
```lua
function main()
    callbacks.add("on_frame", function()
        engine.log_console("tick")
    end)
    return 1
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
void tick_handler()
{
    engine::log_console("tick");
}

int main()
{
    callbacks::add("on_frame", @tick_handler);
    return 1;
}
```
{% endtab %}
{% endtabs %}
