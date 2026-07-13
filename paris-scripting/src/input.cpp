#include "paris/input.hpp"

#include <array>
#include <mutex>
#include <string>

namespace paris::input {

namespace {
    std::mutex g_mtx;
    FrameSnapshot g_snap;
    std::function<bool(int)> g_os_query;

    // VK code → readable name. Only the common ones — the full table would be
    // enormous; the host can extend by wrapping `vk_name` later.
    const char* vk_name_lookup(int vk) {
        switch (vk) {
            case 0x01: return "LMB";       case 0x02: return "RMB";
            case 0x04: return "MMB";       case 0x05: return "MB4";
            case 0x06: return "MB5";       case 0x08: return "Backspace";
            case 0x09: return "Tab";       case 0x0D: return "Enter";
            case 0x10: return "Shift";     case 0x11: return "Ctrl";
            case 0x12: return "Alt";       case 0x14: return "CapsLock";
            case 0x1B: return "Esc";       case 0x20: return "Space";
            case 0x21: return "PgUp";      case 0x22: return "PgDn";
            case 0x23: return "End";       case 0x24: return "Home";
            case 0x25: return "Left";      case 0x26: return "Up";
            case 0x27: return "Right";     case 0x28: return "Down";
            case 0x2D: return "Insert";    case 0x2E: return "Delete";
            default:   return nullptr;
        }
    }
}

void begin_frame(const FrameSnapshot& snap) {
    std::lock_guard lk(g_mtx);
    g_snap = snap;
}

void set_os_key_query(std::function<bool(int)> q) {
    std::lock_guard lk(g_mtx);
    g_os_query = std::move(q);
}

namespace mouse {
    vector2 get_position()         { std::lock_guard lk(g_mtx); return g_snap.mouse_pos; }
    vector2 get_position_desktop() { std::lock_guard lk(g_mtx); return g_snap.mouse_desktop; }
    vector2 get_delta()            { std::lock_guard lk(g_mtx); return g_snap.mouse_delta; }
    float   wheel_delta()          { std::lock_guard lk(g_mtx); return g_snap.wheel; }

    bool hover_rect(const vector2& mn, const vector2& mx) {
        std::lock_guard lk(g_mtx);
        auto& p = g_snap.mouse_pos;
        return p.x >= mn.x && p.x <= mx.x && p.y >= mn.y && p.y <= mx.y;
    }

    bool is_down(int vk)    { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_down[vk]; }
    bool is_pressed(int vk) { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_pressed[vk]; }
}

namespace keyboard {
    bool is_down(int vk)      { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_down[vk]; }
    bool is_pressed(int vk)   { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_pressed[vk]; }
    bool is_released(int vk)  { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_released[vk]; }
    bool is_toggle_on(int vk) { std::lock_guard lk(g_mtx); return vk >= 0 && vk < 256 && g_snap.keys_toggle[vk]; }

    bool is_os_down(int vk) {
        std::lock_guard lk(g_mtx);
        return g_os_query ? g_os_query(vk) : false;
    }

    std::vector<int> get_pressed_keys() {
        std::lock_guard lk(g_mtx);
        std::vector<int> out;
        for (int i = 0; i < 256; ++i) if (g_snap.keys_down[i]) out.push_back(i);
        return out;
    }

    std::string vk_name(int vk) {
        if (auto* n = vk_name_lookup(vk); n) return n;
        // 0x30-0x39 = '0'-'9', 0x41-0x5A = 'A'-'Z'
        if (vk >= 0x30 && vk <= 0x39) return std::string(1, char('0' + vk - 0x30));
        if (vk >= 0x41 && vk <= 0x5A) return std::string(1, char('A' + vk - 0x41));
        if (vk >= 0x70 && vk <= 0x7B) return "F" + std::to_string(vk - 0x6F);
        return "VK_" + std::to_string(vk);
    }
}

namespace text {
    std::string get_recent() {
        std::lock_guard lk(g_mtx);
        std::string s = g_snap.recent_text;
        g_snap.recent_text.clear();
        return s;
    }

    void push_char(char32_t cp) {
        std::lock_guard lk(g_mtx);
        // Naive UTF-32→UTF-8 for BMP codepoints. Extend if you need supplementary planes.
        if (cp < 0x80) g_snap.recent_text.push_back(char(cp));
        else if (cp < 0x800) {
            g_snap.recent_text.push_back(char(0xC0 | (cp >> 6)));
            g_snap.recent_text.push_back(char(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            g_snap.recent_text.push_back(char(0xE0 | (cp >> 12)));
            g_snap.recent_text.push_back(char(0x80 | ((cp >> 6) & 0x3F)));
            g_snap.recent_text.push_back(char(0x80 | (cp & 0x3F)));
        } else {
            g_snap.recent_text.push_back(char(0xF0 | (cp >> 18)));
            g_snap.recent_text.push_back(char(0x80 | ((cp >> 12) & 0x3F)));
            g_snap.recent_text.push_back(char(0x80 | ((cp >> 6) & 0x3F)));
            g_snap.recent_text.push_back(char(0x80 | (cp & 0x3F)));
        }
    }
}

} // namespace paris::input
