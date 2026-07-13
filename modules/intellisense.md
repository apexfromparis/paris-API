# intellisense

Provide completion candidates and hover tooltips for any language. Extensions
declare `on_completion` and `on_hover` at the top level; paris auto-registers
them.

## `on_completion(file, line_text, col)`

Called when the IDE requests completions. Return a list of `{ label, insert, detail }`
tables. Return `{}` to opt out.

- `file` — the currently active file path.
- `line_text` — the text of the current line up to the cursor.
- `col` — 0-based cursor column.

```lua
local snippets = {
    { label = "TODO",  insert = "TODO: ",  detail = "task marker" },
    { label = "FIXME", insert = "FIXME: ", detail = "known bug" },
}

function on_completion(file, line_text, col)
    if not line_text:match("%-%-") and not line_text:match("//") then
        return {}
    end
    return snippets
end
```

## `on_hover(file, word, line)`

Called when the user hovers over a token. Return a markdown string, or `""` to
opt out.

- `file` — the current file path.
- `word` — the token under the cursor.
- `line` — 0-based line number.

```lua
function on_hover(file, word, line)
    if word == "TODO" then
        return "**TODO** — task marker"
    end
    return ""
end
```

## Provider merging

Multiple extensions can each register their own providers. Completions from
all providers are concatenated in registration order. For hover, the first
non-empty result wins.

Both providers are dropped automatically when the extension unloads.
