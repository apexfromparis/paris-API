# Modules

Everything that isn't UI lives in a namespaced module. Scripts access them by
their public name — `render.*`, `editor.*`, `ai.*`, and so on.

## Overlay modules

For scripts that render in-game or hook the input pipeline.

| Module                    | Coverage      | Purpose |
|---------------------------|---------------|---------|
| [render](render.md)       | Lua + AS core | Draw shapes, text, bitmaps. Fonts, effects, clipping. |
| [input](input.md)         | Lua + AS core | Mouse, keyboard, typed text. Frame-based. |
| [mathx](mathx.md)         | Lua + AS core | Constants, `clamp`, `lerp`, `smoothstep`, `remap`, `wrap`, matrix factories. |
| [engine](engine.md)       | Lua + AS      | Logging (overlay + console), user identity. |
| [callbacks](callbacks.md) | Lua + AS      | Register arbitrary event handlers. |

## IDE extension modules

For code-editor-style extensions that hook the editor, AI pipeline, or settings
panel. Extensions use the `on_activate` / `on_tick` / `on_deactivate` lifecycle
rather than `main` / `on_frame` / `on_unload`.

| Module                         | Coverage      | Purpose |
|--------------------------------|---------------|---------|
| [editor](editor.md)            | Lua + AS      | Buffer, cursor, tabs, selection, file events. |
| [clipboard](clipboard.md)      | Lua + AS      | System clipboard read/write. |
| [intellisense](intellisense.md)| Lua           | Register completion + hover providers. |
| [ai](ai.md)                    | Lua + AS core | Pipeline hooks: before send, after response, tool calls, system inject. |
| [tools](tools.md)              | Lua + AS      | Register tools the AI model can call. |
| [settings](settings.md)        | Lua + AS      | Persistent key/value + widget factories for the Settings tab. |
| [terminal](terminal.md)        | Lua + AS      | Spawn integrated terminal tabs, drive them programmatically. |

## Shared modules

Used by either flavour of script.

| Module              | Coverage      | Purpose |
|---------------------|---------------|---------|
| [fs](fs.md)         | Lua + AS core | Sandboxed filesystem under `Documents\My Games\paris`. |
| [util](util.md)     | Lua + AS core | Base64, hex, URL encoding, FNV-1a. |
| [json](json.md)     | Lua + AS core | Encode / decode JSON. |
| [net](net.md)       | Lua + AS core | HTTP GET/POST, WebSockets. |

Modules marked "core" in AngelScript expose the essentials. Modules marked
"Lua" are Lua-only; use Lua for scripts that need them.
