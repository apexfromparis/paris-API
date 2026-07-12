# engine

Logging and user identity. Two log destinations: the on-screen overlay (fades
out) and the persistent debug console.

## Logging

| Function                              | Sink        | Style |
|---------------------------------------|-------------|-------|
| `engine.log(msg)`                     | Overlay     | Info  |
| `engine.log_error(msg)`               | Overlay     | Error |
| `engine.log_console(msg)`             | Console     | Info  |
| `engine.log_console_error(msg)`       | Console     | Error |

Rule of thumb: `log*` for things the user should notice, `log_console*` for
debugging output you'd want in your terminal.

## User identity

| Function                     | Returns  | Purpose |
|------------------------------|----------|---------|
| `engine.get_user_name()`     | `string` | Current paris username. |

## Examples

{% tabs %}
{% tab title="Lua" %}
```lua
engine.log("script loaded")
engine.log_console("verbose init trace")

if some_condition then
    engine.log_error("something is wrong")
end
```
{% endtab %}
{% tab title="AngelScript" %}
```cpp
engine::log("script loaded");
engine::log_console("verbose init trace");
engine::log_error("something is wrong");
engine::log("hi " + engine::get_user_name());
```
{% endtab %}
{% endtabs %}
