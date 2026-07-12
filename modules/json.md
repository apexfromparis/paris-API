# json

Encode Lua tables into JSON and parse JSON strings into Lua tables.

Not currently exposed in AngelScript.

## Functions

| Function                        | Returns  | Purpose |
|---------------------------------|----------|---------|
| `json.parse(text)`              | `any?`   | Parse a JSON string. `nil` on failure. |
| `json.decode(text)`             | `any?`   | Alias for `parse`. |
| `json.stringify(value)`         | `string` | Encode a Lua value. Nested tables supported. |
| `json.encode(value)`            | `string` | Alias for `stringify`. |

## Encoding rules

* Lua tables with sequential integer keys starting at `1` encode as arrays.
* All other tables encode as objects with string keys.
* `nil` values are omitted.
* Numbers, strings, booleans encode directly.
* Functions, userdata, and other non-JSON types are dropped with a warning.

## Decoding rules

* JSON objects become tables with string keys.
* JSON arrays become tables with integer keys starting at `1`.
* JSON `null` becomes Lua `nil`.

## Example — round-trip a config

```lua
local settings = {
    fov = 90,
    color = { r = 0, g = 255, b = 120, a = 255 },
    binds = { "F1", "F2", "F3" }
}

local encoded = json.stringify(settings)
fs.create_file("settings.json", encoded)

local ok, raw = fs.read_file("settings.json")
if ok then
    local decoded = json.decode(raw)
    engine.log(string.format("fov = %d", decoded.fov))
end
```
