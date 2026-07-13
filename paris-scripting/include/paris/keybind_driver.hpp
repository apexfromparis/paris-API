#pragma once

namespace paris::keybind_driver {

// Walks every keybind element in the UI tree and updates its `keybind_state`
// based on the current input snapshot + the element's mode. Call this once
// per frame, after `input::begin_frame` and before `engine.tick()`.
//
// When a keybind lives under a checkbox and its mode expresses intent
// (`On`, `Toggle`, `AlwaysOn`), the parent checkbox's value is mirrored to
// the keybind state so scripts reading the checkbox see the hotkey effect
// without touching the keybind directly.
void tick();

} // namespace paris::keybind_driver
