# fs

Sandboxed filesystem. Every path is relative and gets reprojected under
`Documents\My Games\paris`. Absolute paths and `..` traversal are rejected.

Not currently exposed in AngelScript.

## Functions

| Function                                         | Returns              | Purpose |
|--------------------------------------------------|----------------------|---------|
| `fs.create_file(path, data)`                     | `bool`               | Write / overwrite. Creates parent directories. |
| `fs.create_directory(path)`                      | `bool`               | Make an empty directory. |
| `fs.read_file(path)`                             | `(bool, string)`     | Returns `(ok, content)`. On failure `ok = false` and content is empty. |
| `fs.does_file_exist(path)`                       | `bool`               | Check existence. |
| `fs.delete_file(path)`                           | `bool`               | Remove a file. |
| `fs.delete_directory(path)`                      | `bool`               | Recursive remove. |
| `fs.query_directory(path, include_dirs?, include_files?, {".ext"...}?)` | array of `{name, is_directory, size}` | List a directory with optional filtering. |

Defaults: `include_dirs = true`, `include_files = true`, no extension filter.

## Path rules

* Must be relative.
* No `..` components.
* Forward or backward slashes both accepted; internally normalised to `/`.
* Directories in the path are created lazily by `create_file`.

## Example — cache a downloaded file

```lua
local ok, body = false, ""

panel:add_button("Fetch config", function()
    local http_ok, code, resp = net.http_get("https://example.com/config.json", 5000)
    if not http_ok or code ~= 200 then
        engine.log_error("fetch failed")
        return
    end
    fs.create_directory("cache")
    fs.create_file("cache/latest.json", resp)
    engine.log("cached " .. #resp .. " bytes")
end)

panel:add_button("Load cached", function()
    if not fs.does_file_exist("cache/latest.json") then
        engine.log_error("no cache")
        return
    end
    local read_ok, blob = fs.read_file("cache/latest.json")
    if read_ok then engine.log("loaded " .. #blob .. " bytes") end
end)
```

## Example — enumerate configs

```lua
local configs = fs.query_directory("configs", false, true, { ".cfg" })
for _, e in ipairs(configs) do
    engine.log_console(string.format("- %s (%d bytes)", e.name, e.size))
end
```
