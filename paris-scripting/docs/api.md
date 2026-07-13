# paris-scripting API reference

Every module is exposed in both Lua and AngelScript under the same names. Where
a signature differs between languages (e.g. AngelScript needs handle syntax
`Type@`), both variants are shown.

Where AS coverage is partial, the row is marked `(Lua only)`.

---

## Lifecycle

Scripts define up to three top-level functions.

| Function        | Required | Return | Semantics |
|-----------------|----------|--------|-----------|
| `main()`        | yes      | `int`  | Ran once at load. Return `> 0` **and** define `on_frame` for the script to persist. |
| `on_frame()`    | no       | `int`  | Ran every frame. Return `<= 0` to unload the script. |
| `on_unload()`   | no       | `void` | Ran once when the script is dropped. |

```lua
function main()      return 1 end
function on_frame()  return 1 end
function on_unload() end
```

```angelscript
int  main()       { return 1; }
int  on_frame()   { return 1; }
void on_unload()  {}
```

---

## Types

### `vector2` / `vec2_t`

Fields: `x`, `y` (float).

| Method          | Signature |
|-----------------|-----------|
| `new`           | `vector2.new()`, `vector2.new(x, y)` |
| `length`        | `float length()` |
| `length_sqr`    | `float length_sqr()` |
| `distance`      | `float distance(other)` |
| `dot`           | `float dot(other)` |
| `normalized`    | `vector2 normalized()` |
| `to_string`     | `string to_string()` |
| operators       | `+`, `-`, `* scalar` |

### `vector3` / `vec3_t`

Fields: `x`, `y`, `z` (float).

Adds `cross(other) -> vector3` and `to_screen()` — Lua returns `vector2` or
`nil`, AngelScript takes an `out` parameter and returns `bool`.

### `color_t`

Fields: `r`, `g`, `b`, `a` (int 0–255).

| Method            | Notes |
|-------------------|-------|
| `color_t.new(r, g, b, a?)` | alpha defaults to 255 |
| `with_alpha(a)`   | returns a copy |
| `lerp(other, t)`  | linear interp per channel |
| `color_t.from_hsv(h, s, v)` | h/s/v in `[0, 1]`, returns opaque color |

### `quaternion`

Fields: `x`, `y`, `z`, `w`.

Methods: `rotate(vec3)`, `inverse()`, `to_euler() -> vec3`, `operator*`.
Statics: `quaternion.identity()`, `quaternion.from_euler(rx, ry, rz)`.

### `matrix4x4`

Row-major. Access cells with `at(row, col)` / `at(row, col, value)`.
Methods: `transform(vec3)`, `operator*`.

`mat4.identity() / translation(v) / scaling(v) / rotation(q)` return factories.

---

## `ui` — hierarchical menu builder

Top-level: five tabs indexed 0–4. Each script creates one or more **subtabs**,
each subtab holds **panels**, each panel holds interactive **elements**.

### Subtab

```lua
local sub = ui.create_subtab(tab_index, name)      -- tab_index in [0, 4]
local pan = sub:add_panel(name, is_small)          -- is_small changes width
sub:set_active(bool)                                -- hide/show subtab
```

```angelscript
Subtab@ sub = ui::create_subtab(0, "ESP");
Panel@  pan = sub.add_panel("Player", false);
```

### Panel — element factories

Signatures (Lua on top, AS underneath where covered):

| Element         | Signature |
|-----------------|-----------|
| `add_checkbox`  | `add_checkbox(name, initial, draw_title=true, find_protect=false, draw_just_label=false) -> Checkbox` |
| `add_slider_int`| `add_slider_int(name, postfix, value, min, max, step, draw_title=true, find_protect=false) -> SliderInt` |
| `add_slider_double` | `add_slider_double(name, postfix, value, min, max, step, ...) -> SliderDouble` |
| `add_input`     | `add_input(name, initial_text, ...) -> Input` (Lua only) |
| `add_multi_select` | `add_multi_select(name, {options}, is_expandable, ...) -> MultiSelect` (Lua only) |
| `add_single_select` | `add_single_select(name, {options}, initial_index, is_expandable, ...) -> SingleSelect` (Lua only) |
| `add_keybind`   | `add_keybind(parent_checkbox, name, vk, mode, ...) -> Keybind` (Lua only). Mode is `"off" \| "on" \| "single" \| "toggle" \| "always_on"`. |
| `add_color`     | `add_color(parent_checkbox, name, {r, g, b, a?}, find_protect=false) -> ColorPicker` |
| `add_button`    | `add_button(name, callback) -> Button` |
| `add_list`      | `add_list(name, {rows}, ...) -> List` (Lua only). Each row is `{ name = "...", info = "..." }`. |

Colour pickers and keybinds must be attached to a parent checkbox. Perception
enforces a limit of two of these per checkbox; this implementation doesn't
enforce it — it will accept more but the menu renderer should treat the first
two as the visible slots.

### Element handles

Every element returns a handle with `get()`, some with `set(...)`, all with
`set_active(bool)`:

| Handle          | `get()` returns         | `set(...)` |
|-----------------|-------------------------|------------|
| Checkbox        | `bool`                  | `bool` |
| SliderInt       | `int`                   | `int` |
| SliderDouble    | `double`                | `double` |
| Input           | `string`                | `string` |
| MultiSelect     | `{bool...}`             | `(index, bool)` |
| SingleSelect    | `int` (0-indexed)       | `int` |
| Keybind         | `is_pressed() -> bool`  | `(vk, mode)` |
| ColorPicker     | `color_t`               | `color_t` |
| Button          | —                       | `(none)` |
| List            | `int` (active index)    | see next table |

List handle extras:

`append(name, info)` · `append_after(name, info, idx)` · `get_all()` ·
`get_count()` · `clear()` · `highlight(idx)` · `remove_highlight(idx)` ·
`remove(idx)` · `set_active_index(idx)`.

### Top-level UI functions

| Function            | Purpose |
|---------------------|---------|
| `ui.create_subtab(idx, name)` | Create a subtab under a top-level tab. |
| `ui.find_element(tab_idx, subtab, panel, element, type)` | Look up an element created by another script. Type must be one of `"checkbox"`, `"slider_int"`, `"slider_double"`, `"input"`, `"multi_select"`, `"single_select"`, `"keybind"`, `"color_picker"`, `"button"`, `"list"`. Returns the raw Element or nil. |
| `ui.is_active()`    | `true` when the menu is on-screen. |
| `ui.construct_config()` | Serialise all UI state into a line-based text blob. |
| `ui.apply_config(blob)` | Restore state from a previously-serialised blob. |

---

## `render` — draw calls

### Metrics

- `render.get_view() -> vector2` — viewport size in overlay pixels.
- `render.get_view_scale() -> float` — DPI multiplier (1.0 = 100%).
- `render.get_fps() -> float` — current frame rate.

### Shapes

Every drawing call accepts a `color_t`. `thickness` and `rounding` default to
`1.0` / `0.0` when omitted.

- `draw_rect(min, max, color, t?, rnd?)`
- `draw_rect_filled(min, max, color, rnd?)`
- `draw_gradient(min, max, top_left, top_right, bottom_right, bottom_left)`
- `draw_line(a, b, color, t?)`
- `draw_circle(center, radius, color, t?, segments?)`
- `draw_circle_filled(center, radius, color, segments?)`
- `draw_arc(center, radius, from_rad, to_rad, color, t?, segments?)`
- `draw_triangle(a, b, c, color, t?)`
- `draw_triangle_filled(a, b, c, color)`
- `draw_polygon({points}, color, t?)`
- `draw_polygon_filled({points}, color)`

### Text

Constants: `render.FONT_18`, `render.FONT_20`, `render.FONT_24`, `render.FONT_28`.
Effects: `render.EFFECT_NONE`, `render.EFFECT_OUTLINE`, `render.EFFECT_SHADOW`,
`render.EFFECT_GLOW`.

- `draw_text(pos, color, text, font?, effect?, effect_color?)`
- `get_text_size(text, font?) -> { w, h }`
- `get_char_advance(codepoint, font?) -> float`
- `load_font_from_memory(ttf_bytes, pixel_size) -> font_id`
- `load_font_from_file(path, pixel_size) -> font_id`

### Bitmaps

- `create_bitmap(rgba8_bytes, width, height) -> id`
- `destroy_bitmap(id)`
- `draw_bitmap(id, min, max, tint?, rnd?)`

### Clipping

- `clip_push(min, max)` — subsequent draw calls are clipped to this rect.
- `clip_pop()` — restore the previous clip stack level.

---

## `input`

All state is frame-based — the host snapshots it into `paris::input::FrameSnapshot`
before calling `engine.tick()`.

### `input.mouse`

- `get_position() -> vector2` — overlay-relative
- `get_position_desktop() -> vector2` — absolute screen coords
- `get_delta() -> vector2` — movement since last frame
- `wheel_delta() -> float` — notches, positive up
- `hover_rect(min, max) -> bool`
- `is_down(vk_button)` / `is_pressed(vk_button)` — for mouse buttons

### `input.keyboard`

- `is_down(vk)` — held
- `is_pressed(vk)` — just went down this frame
- `is_released(vk)` — just went up this frame
- `is_toggle_on(vk)` — caps-lock-style toggle
- `is_os_down(vk)` — raw GetAsyncKeyState bypass
- `get_pressed_keys() -> {vk...}`
- `vk_name(vk) -> string`

### `input.text` (Lua only)

- `get_recent() -> string` — UTF-8 text typed since the last call; drains the buffer.

---

## `fs` (Lua only)

Sandboxed. All paths are relative to `Documents\My Games\paris` (override with
`paris::fs::set_root(...)` on the C++ side).

- `fs.create_file(rel, data)` — writes/overwrites
- `fs.create_directory(rel)`
- `fs.read_file(rel)` — returns `(ok, data)`
- `fs.does_file_exist(rel)`
- `fs.delete_file(rel)`
- `fs.delete_directory(rel)`
- `fs.query_directory(rel, include_dirs?, include_files?, {".ext"...}?)` — returns array of `{ name, is_directory, size }`

---

## `mathx`

Constants: `M_PI`, `M_TAU`, `DEG2RAD`, `RAD2DEG`.

Helpers: `clamp(v, min, max)` · `saturate(v)` · `lerp(a, b, t)` ·
`smoothstep(edge0, edge1, x)` · `remap(v, in_min, in_max, out_min, out_max)` ·
`wrap(v, min, max)` · `inverse_lerp(a, b, v)`.

Matrix factories: `mat4.identity()` · `mat4.translation(v)` · `mat4.scaling(v)` ·
`mat4.rotation(q)`.

---

## `util` (Lua only)

Byte-safe encoders. Decoders return `nil` on failure.

- `base64_encode(bytes) -> string` · `base64_decode(text) -> string?`
- `hex_encode(bytes) -> string` (uppercase) · `hex_decode(text) -> string?`
- `url_encode(bytes) -> string` · `url_decode(text) -> string` (does not translate `+` to space)
- `fnv1a64(bytes) -> number` — non-cryptographic 64-bit hash

---

## `json` (Lua only)

- `json.parse(text)` / `json.decode(text)` — returns a re-serialised string on
  success, `nil` on failure. Use a native Lua JSON library on top if you need
  typed table access.
- `json.stringify(table)` / `json.encode(table)` — encodes a shallow Lua table.

---

## `net` (Lua only, Windows via WinHTTP)

HTTP:

- `net.http_get(url, timeout_ms?) -> (ok, status, body)`
- `net.http_post(url, content_type, body, timeout_ms?) -> (ok, status, body)`

WebSockets: `ws_connect(url, timeout_ms?)` returns `(ws, err)`. Handle methods:
`is_open`, `send_text`, `send_binary`, `send_json`, `recv`, `poll`, `close`.
Only text/binary/close frame types are surfaced.

Timeouts default to 15s if omitted.

---

## `engine`

- `engine.log(msg)` — top-left overlay notification, auto-fade
- `engine.log_error(msg)` — same, styled as error
- `engine.log_console(msg)` — persistent debug console
- `engine.log_console_error(msg)`
- `engine.get_user_name() -> string`

---

## `callbacks`

- `callbacks.add(event, function)` — register a callback for any event name.
  Reserved by the engine: `"on_frame"`, `"on_unload"`. Emit your own via
  `paris::callbacks::fire("your_event")` on the host side.

---

## Sandboxing

Neither language runtime enforces a wall-clock timeout by default. If you ship
scripts written by third parties:

- **Lua** — install `lua_sethook(L, hook, LUA_MASKCOUNT, N)` and raise a
  `luaL_error` from your hook after N instructions.
- **AngelScript** — call `SetLineCallback` on the context and abort with
  `Abort()` when your budget is exhausted.

The Lua sandbox already excludes `io`, `debug`, `package`, and other unsafe
libraries — only `base`, `math`, `string`, `table`, `os`, `coroutine`, `utf8`
are opened. To lock this down further, override `os.execute`, `os.remove`,
`os.rename` in your fork after `open_libraries`.

---

## Extending

The C++ API is deliberately modular. Adding a new module `foo`:

1. Create `include/paris/foo.hpp` and `src/foo.cpp`.
2. Add `src/foo.cpp` to `CMakeLists.txt`.
3. In `src/script_lua.cpp`, add a `bind_foo()` method calling
   `lua_["foo"].get_or_create<sol::table>()` and register functions.
4. In `src/script_as.cpp`, add a `RegisterFoo(engine)` function and call it
   from `get_or_init_as_engine()`.

Adding a new UI element kind:

1. Extend `ElementKind` in `include/paris/ui.hpp`.
2. Add a variant slot in `Element::value` if the element carries state.
3. Add a handle class + `Panel::add_*` method.
4. Bind it in both script backends.
5. Handle the new kind in your menu renderer's switch statement.
