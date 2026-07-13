#pragma once

#include <functional>
#include <string>

namespace paris::clipboard {

// Returns "" when no backend is installed or the clipboard is empty.
std::string get();
void        set(const std::string& text);

struct Backend {
    std::function<std::string()>     get;
    std::function<void(std::string)> set;
};

void install(Backend b);

} // namespace paris::clipboard
