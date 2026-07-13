# clipboard

System clipboard read + write.

| Function                    | Returns  |
|-----------------------------|----------|
| `clipboard.get()`           | `string` |
| `clipboard.set(text)`       | none     |

## Example

```lua
settings.create_button("Copy file path", function()
    clipboard.set(editor.get_active_file())
    editor.show_notification("path copied")
end)
```
