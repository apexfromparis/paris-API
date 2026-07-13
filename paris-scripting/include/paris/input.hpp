#pragma once

#include "types.hpp"

#include <functional>
#include <string>
#include <vector>

// The Input API is frame-based: each function reads the state as of the last
// call to `input::update()`, which the host runs once per frame before
// dispatching the `on_frame` event.

namespace paris::input {

// ---------- Mouse ----------

namespace mouse {
    vector2 get_position();             // relative to the overlay
    vector2 get_position_desktop();     // absolute screen coords
    vector2 get_delta();                // frame-to-frame movement
    float   wheel_delta();              // sign = direction, magnitude = notches
    bool    hover_rect(const vector2& min, const vector2& max);

    bool    is_down(int vk_button);     // VK_LBUTTON / VK_RBUTTON / VK_MBUTTON / VK_XBUTTON1 / VK_XBUTTON2
    bool    is_pressed(int vk_button);  // one-shot
}

// ---------- Keyboard ----------

namespace keyboard {
    bool     is_down(int vk);       // held
    bool     is_pressed(int vk);    // just went down this frame
    bool     is_released(int vk);   // just went up this frame
    bool     is_toggle_on(int vk);  // caps-lock style toggle
    bool     is_os_down(int vk);    // raw GetAsyncKeyState — bypasses UI focus

    std::vector<int> get_pressed_keys();
    std::string      vk_name(int vk);
}

// ---------- Text ----------

namespace text {
    // Returns characters typed since the last call. Clears the buffer.
    std::string get_recent();
    void        push_char(char32_t cp); // host-side: called from your WndProc / IME sink.
}

// ---------- Frame boundary ----------

// Called by the host at the top of each frame — snapshots the current input
// state so scripts see a stable view for the whole frame.
struct FrameSnapshot {
    vector2 mouse_pos;
    vector2 mouse_desktop;
    vector2 mouse_delta;
    float   wheel = 0.f;
    bool    keys_down[256]     {};
    bool    keys_pressed[256]  {};
    bool    keys_released[256] {};
    bool    keys_toggle[256]   {};
    std::string recent_text;
};

void begin_frame(const FrameSnapshot& snap);

// Optional wiring: pass in raw OS-level "is this VK held right now" for the
// bypass query. Defaults to always false.
void set_os_key_query(std::function<bool(int)> q);

} // namespace paris::input
