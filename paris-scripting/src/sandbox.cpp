#include "paris/sandbox.hpp"

#include <atomic>
#include <chrono>

namespace paris::sandbox {

namespace {
    std::atomic<int> g_budget_ms{ 0 };   // 0 = disabled
    std::atomic<int> g_step     { 1000 };

    thread_local std::chrono::steady_clock::time_point tl_start;
}

void set_frame_budget_ms(int ms) { g_budget_ms.store(ms < 0 ? 0 : ms); }
int  get_frame_budget_ms()       { return g_budget_ms.load(); }

void set_lua_instruction_step(int n) { g_step.store(n < 1 ? 1 : n); }
int  get_lua_instruction_step()      { return g_step.load(); }

void begin_hook() { tl_start = std::chrono::steady_clock::now(); }

bool over_budget() {
    int b = g_budget_ms.load();
    if (b <= 0) return false;
    auto now = std::chrono::steady_clock::now();
    auto dt  = std::chrono::duration_cast<std::chrono::milliseconds>(now - tl_start).count();
    return int(dt) >= b;
}

} // namespace paris::sandbox
