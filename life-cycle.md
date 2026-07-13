# Life cycle

paris supports two script flavours with different lifecycle contracts:

* **Overlay scripts** — hook the game render / input pipeline. Use `main` /
  `on_frame` / `on_unload`.
* **IDE extensions** — hook the editor / AI / settings. Use `on_activate` /
  `on_tick` / `on_deactivate`.

You pick a flavour by which functions you define. If a script mixes both,
paris runs `main` and `on_activate` in that order; `on_tick` takes priority
over `on_frame` for the per-frame slot.

## Overlay scripts

### Required: `main`

Runs once at load. Returns an int:

| Return | + `on_frame`? | Result |
|--------|---------------|--------|
| `> 0`  | yes           | Script persists, `on_frame` starts firing. |
| `> 0`  | no            | Dropped immediately after `main`. |
| `<= 0` | any           | Dropped immediately after `main`. |

### Optional: `on_frame`

Called every render tick. Return `> 0` to keep running, `<= 0` to unload.

### Optional: `on_unload`

Called once, right before the script is dropped.

## IDE extensions

### Required: `on_activate`

Runs once when the extension loads. Set up UI subtabs / settings widgets,
register tools, initialise state. Return value ignored — persistence is
implicit for IDE extensions.

### Optional: `on_tick`

Called every frame on the main thread. Cheap work only — don't block.

### Optional: event hooks

Define any subset of these as top-level functions. paris auto-wires them.

| Function                                | Fires when |
|-----------------------------------------|-----------|
| `on_file_opened(path)`                  | File becomes active. |
| `on_file_saved(path)`                   | User saves. |
| `on_buffer_changed(path, line)`         | Buffer content changed on that line. |
| `on_tab_changed(path)`                  | Active tab changed. |
| `on_settings_render(x, y, w)`           | Extension's Settings tab is drawn. |
| `on_completion(file, line, col)`        | IntelliSense completion request. |
| `on_hover(file, word, line)`            | Hover tooltip request. |
| `on_ai_before_send(prompt, system)`     | Just before an AI request goes out. |
| `on_ai_after_response(response)`        | Model returned. |
| `on_ai_tool_call(name, args_json)`      | Model called a tool. |
| `on_ai_after_tool(name, args, result)`  | Any tool call resolved. |
| `on_ai_system_inject()`                 | Building the system prompt. |

### Optional: `on_deactivate`

Runs once before the extension is dropped. Undo any external side effects
(open terminals, registered tools) here.

## Warnings

{% hint style="warning" %}
`on_frame` and `on_tick` are hot paths. Avoid:
* Building UI or Settings widgets
* Blocking network / disk calls
* Allocating large temporary tables
{% endhint %}

{% hint style="warning" %}
Create your controls (overlay UI or IDE settings widgets) exactly once —
overlay scripts in `main`, IDE extensions in `on_activate`. Building widgets
every frame will freeze the menu.
{% endhint %}

## Full skeletons

### Overlay script

{% tabs %}
{% tab title="Lua" %}
```lua
local subtab, panel, enabled

function main()
    subtab  = ui.create_subtab(0, "My Script")
    panel   = subtab:add_panel("Settings", false)
    enabled = panel:add_checkbox("Enable", false)
    return 1
end

function on_frame()
    if not enabled:get() then return 1 end
    return 1
end

function on_unload() end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
Subtab@   subtab;
Panel@    panel;
Checkbox@ enabled;

int main()
{
    @subtab  = ui::create_subtab(0, "My Script");
    @panel   = subtab.add_panel("Settings", false);
    @enabled = panel.add_checkbox("Enable", false);
    return 1;
}

int on_frame() { return 1; }

void on_unload() {}
```
{% endtab %}
{% endtabs %}

### IDE extension

{% tabs %}
{% tab title="Lua" %}
```lua
function on_activate()
    editor.show_notification("my extension loaded")
end

function on_file_saved(path)
    editor.set_status("saved: " .. path)
end

function on_settings_render(x, y, w)
    settings.create_checkbox("Enabled", "enabled")
end

function on_deactivate() end
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
void on_activate()
{
    editor::show_notification("my extension loaded");
}

void on_file_saved(const string &in path)
{
    editor::set_status("saved: " + path);
}

void on_deactivate() {}
```
{% endtab %}
{% endtabs %}

## Hot reload

paris watches your scripts directory. When a file changes on disk:

1. The old instance runs `on_deactivate` (or `on_unload` for overlay scripts)
2. Its UI subtree, callbacks, tools, providers, and widgets are dropped
3. The new source is loaded and `on_activate` / `main` runs

You don't have to restart paris — just save the file.
