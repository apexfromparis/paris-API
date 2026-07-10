---
description: Official scripting API for paris.
---

# Introduction

Welcome to the **paris scripting API**. This documentation covers everything you
need to write scripts for paris in **AngelScript** and **Lua** — from core types
like [`vec2_t`](types/vec2_t.md) to the full [GUI API](gui-api/README.md).

Every page shows both languages side by side. Pick the tab for the language you
write in:

{% tabs %}
{% tab title="Lua" %}
```lua
local pos = vec2_t.new(100, 200)
gui.text("hello from " .. tostring(pos))
```
{% endtab %}

{% tab title="AngelScript" %}
```cpp
vec2_t pos(100, 200);
gui::text("hello from " + pos.to_string());
```
{% endtab %}
{% endtabs %}

## Sections

* [Getting started](getting-started.md) — load your first script.
* [Types](types/vec2_t.md) — `vec2_t`, `vec3_t`, `color_t`, …
* [GUI API](gui-api/README.md) — build menu controls for your script.

{% hint style="info" %}
Scripts are published in the **Script Market** on the forum by certified
developers. Ask an admin for the `developper` role to publish yours.
{% endhint %}
