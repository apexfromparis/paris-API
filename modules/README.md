# Modules

Everything that isn't UI lives in a namespaced module. Scripts access them by
their public name — `render.*`, `input.*`, `fs.*`, and so on.

| Module                    | Coverage      | Purpose |
|---------------------------|---------------|---------|
| [render](render.md)       | Lua + AS core | Draw shapes, text, bitmaps. Fonts, effects, clipping. |
| [input](input.md)         | Lua + AS core | Mouse, keyboard, typed text. Frame-based. |
| [fs](fs.md)               | Lua           | Sandboxed filesystem under `Documents\My Games\paris`. |
| [mathx](mathx.md)         | Lua + AS core | Constants, `clamp`, `lerp`, `smoothstep`, `remap`, `wrap`, matrix factories. |
| [util](util.md)           | Lua           | Base64, hex, URL encoding, FNV-1a. |
| [json](json.md)           | Lua           | Encode / decode JSON. |
| [net](net.md)             | Lua           | HTTP GET/POST, WebSockets. |
| [engine](engine.md)       | Lua + AS      | Logging (overlay + console), user identity. |
| [callbacks](callbacks.md) | Lua + AS      | Register arbitrary event handlers. |

Modules marked "core" in AngelScript expose the essentials — enough for
overlays and UI. If a script needs `fs`, `json`, or `net`, prefer Lua for now.
