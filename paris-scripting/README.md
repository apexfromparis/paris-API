# paris-scripting

An embeddable scripting engine for a Windows usermode client. Loads user
scripts written in **Lua** or **AngelScript**, exposes a single unified API
surface (UI hierarchy, rendering, input, filesystem, math, JSON, HTTP,
WebSockets), and lets your host drive the whole thing via three calls:
`load_directory`, `tick`, `shutdown`.

The API shape follows the [perception.cx](https://docs.perception.cx/perception/lua-script/)
convention: 5 top-level tabs, subtabs, panels, elements. Scripts create their
UI once from `main()`, then read state each frame from `on_frame()`.

## Build

Requires C++17, MSVC or clang on Windows, and [vcpkg](https://github.com/microsoft/vcpkg).

```
git clone https://github.com/microsoft/vcpkg <somewhere>
<somewhere>\bootstrap-vcpkg.bat

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<somewhere>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Dependencies are declared in `vcpkg.json` — vcpkg pulls them automatically:

- `lua`
- `sol2`
- `angelscript`
- `nlohmann-json`

The static library target is `paris_scripting`. A demo host that prints every
draw call to stdout ships as `paris_demo`.

## Integrate into your usermode

Three steps.

**1. Wire the render backend.** Fill in a `paris::render::Backend` with your
existing ImGui/D3D functions, then call `install`:

```cpp
paris::render::Backend b;
b.draw_rect      = [](auto mn, auto mx, auto c, float t, float r) { /* ImGui... */ };
b.draw_text      = [](auto p, auto c, auto s, auto f, auto fx, auto fxc) { /* ... */ };
b.world_to_screen = [](paris::vector3 w, paris::vector2& out) { /* your matrix */ };
// ... fill the rest, unfilled fields are no-ops
paris::render::install(std::move(b));
```

**2. Snapshot input each frame** before ticking the engine:

```cpp
paris::input::FrameSnapshot snap;
snap.mouse_pos     = { mx, my };
snap.mouse_desktop = ...;
for (int vk = 0; vk < 256; ++vk) {
    snap.keys_down[vk]     = held(vk);
    snap.keys_pressed[vk]  = just_pressed(vk);
    snap.keys_released[vk] = just_released(vk);
}
paris::input::begin_frame(snap);
```

**3. Load, tick, shutdown.**

```cpp
paris::ScriptEngine engine;
engine.load_directory("scripts");

while (running) {
    render_frame_start();
    paris::input::begin_frame(build_snapshot());
    engine.tick();          // dispatches on_frame to every live script
    render_frame_end();
}
engine.shutdown();          // dispatches on_unload, cleans up UI/callbacks
```

Your menu iterates `paris::ui::layout()` and renders each element by kind — see
[docs/menu_integration.md](docs/menu_integration.md).

## Script lifecycle

Every script must define at least `main()`. If it returns `> 0` **and** the
script also defines `on_frame()`, it persists; otherwise it's dropped
immediately after `main()`. `on_frame()` runs every frame; returning `<= 0`
unloads the script. `on_unload()` runs once on the way out.

```lua
function main()
    -- set up UI, register callbacks
    return 1
end

function on_frame()
    -- per-frame logic
    return 1
end

function on_unload()
    -- optional cleanup
end
```

AngelScript scripts follow the same pattern with `int main()`, `int on_frame()`,
`void on_unload()`.

## API at a glance

| Module      | Coverage in Lua | Coverage in AngelScript |
|-------------|-----------------|-------------------------|
| Types (`vector2/3`, `color_t`, `quaternion`, `matrix4x4`) | ✅ full | ✅ full |
| `ui`        | ✅ full: subtabs, panels, all 10 element kinds, keybinds, config save/load | ✅ core: checkbox, sliders, button, color picker |
| `render`    | ✅ full: shapes, text w/ effects, fonts, bitmaps, clipping | ✅ core: shapes + text |
| `input`     | ✅ full | ✅ mouse + keyboard basics |
| `fs`        | ✅ full (sandboxed to `Documents\My Games\paris`) | ✅ core (create/read/exists/delete) |
| `mathx`     | ✅ full | ✅ scalar helpers |
| `util`      | ✅ base64 / hex / url / fnv | ✅ base64 / hex / url |
| `json`      | ✅ parse / stringify (deep) | ✅ parse (validate + normalise) |
| `net`       | ✅ HTTP GET/POST + WS (via WinHTTP) | ✅ HTTP GET/POST (string form) |
| `engine`    | ✅ log family + `get_user_name` | ✅ log family + `get_user_name` |
| `callbacks` | ✅ arbitrary events | ✅ arbitrary events |

For the full function-by-function reference, see [docs/api.md](docs/api.md).

## Repo layout

```
paris_scripting/
├── include/paris/           public C++ headers
├── src/                     implementations
├── demo/                    minimal host that prints draw calls
├── scripts/                 example .lua and .as scripts
├── docs/                    API + integration reference
├── CMakeLists.txt
└── vcpkg.json
```

## Optional extras (opt-in headers)

The core library is deliberately minimal. These headers add the batteries and
you drop them in when you want them:

| Header                      | What it gives you |
|-----------------------------|-------------------|
| `paris/hot_reload.hpp`      | `ReadDirectoryChangesW` watcher — save a script, it reloads. |
| `paris/sandbox.hpp`         | Per-frame execution budget for both Lua and AngelScript. Runaway scripts get aborted, not killed. |
| `paris/keybind_driver.hpp`  | `paris::keybind_driver::tick()` — updates every keybind element's state from the input snapshot and mirrors it to the parent checkbox. |
| `paris/autosave.hpp`        | One-liner config save/load hooked to the lifecycle. |
| `paris/metadata.hpp`        | Parse `-- @name / @author / @version / @description` headers from a script file. |
| `paris/imgui_backend.hpp`   | Single-header ImGui backend — wires the render calls to `ImGui::GetForegroundDrawList()` and draws the menu with one call. |

Enabling them from your host:

```cpp
paris::sandbox::set_frame_budget_ms(4);           // abort scripts that run > 4ms

paris::ScriptEngine engine;
engine.load_directory("scripts");

paris::HotReload watcher(engine);
watcher.start("scripts");                         // background thread

paris::autosave::install_hooks();                 // load on first tick, save on shutdown

while (running) {
    paris::input::begin_frame(build_snapshot());
    paris::keybind_driver::tick();                // now every ui::Keybind is live
    engine.tick();
    render_frame();
}
```

## Security notes

- `fs` is sandboxed by construction: absolute paths and `..` traversal are
  rejected. Change the root with `paris::fs::set_root("...")` at boot.
- `net` uses WinHTTP; TLS certificate validation is on by default. Whitelist
  hosts at your firewall if you want to lock scripts down further.
- Scripts run on the same thread as your renderer. A pathological infinite
  loop will freeze the frame — hook `lua_sethook` / `asIScriptContext::SetLineCallback`
  in your fork if you need timeout enforcement (see [docs/api.md § Sandboxing](docs/api.md#sandboxing)).

## License

Whatever your project uses — this is a scaffolding drop, no license imposed.
