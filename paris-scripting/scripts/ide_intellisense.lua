-- @name TODO IntelliSense
-- @author paris-scripting
-- @version 0.1.0
-- @description Adds completion + hover for TODO / FIXME comments.

local snippets = {
    { label = "TODO",    insert = "TODO: ",    detail = "task marker" },
    { label = "FIXME",   insert = "FIXME: ",   detail = "known bug" },
    { label = "NOTE",    insert = "NOTE: ",    detail = "explanatory note" },
    { label = "HACK",    insert = "HACK: ",    detail = "workaround" },
    { label = "SAFETY",  insert = "SAFETY: ",  detail = "invariant / assumption" },
}

function on_activate()
    editor.show_notification("TODO intellisense loaded")
end

-- Return a list of {label, insert, detail} tables. Return an empty list to opt
-- out for this position.
function on_completion(file, line_text, col)
    -- Only fire inside a comment.
    if not line_text:match("%-%-") and not line_text:match("//") then return {} end
    -- Filter by the current prefix — the word to the left of the cursor.
    local prefix = line_text:match("(%w+)$") or ""
    prefix = prefix:upper()
    local out = {}
    for _, s in ipairs(snippets) do
        if s.label:sub(1, #prefix) == prefix then table.insert(out, s) end
    end
    return out
end

function on_hover(file, word, line)
    for _, s in ipairs(snippets) do
        if s.label == word:upper() then return "**" .. s.label .. "** — " .. s.detail end
    end
    return ""
end
