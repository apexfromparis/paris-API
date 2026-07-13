#pragma once

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace paris::callbacks {

// Two flavours of registrable callback:
//   fn_t     — no args, the common case (paint / on_frame / on_unload).
//   fn_any_t — takes a single `std::any` payload the fire site prepares. Use
//              this for events like on_key that carry data.
using fn_t     = std::function<void()>;
using fn_any_t = std::function<void(const std::any&)>;

// Register a plain callback. The engine tags it with the currently-loading
// script's section so it can be dropped when the script unloads.
void add(std::string_view event, fn_t fn);

// Register a callback that receives a typed payload. The fire site fills the
// std::any with the same type; each script decides how to cast it.
void add_any(std::string_view event, fn_any_t fn);

// Dispatch. `fire` covers plain events; `fire_any` covers payload events.
void fire     (std::string_view event);
void fire_any (std::string_view event, const std::any& payload);

// Called by the engine at unload time — removes every callback tagged with the
// given section.
void drop_section(const std::string& section);

namespace detail {
    void push_section(std::string name);
    void pop_section();
    std::string current_section();
}

} // namespace paris::callbacks
