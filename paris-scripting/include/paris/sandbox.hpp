#pragma once

#include <chrono>

namespace paris::sandbox {

// Per-frame budget applied to script execution. Zero disables the timeout.
// When exceeded, the script is aborted with an error; it stays loaded and
// gets another try next frame.
void set_frame_budget_ms(int ms);
int  get_frame_budget_ms();

// Called by the script backends at the start of every hook invocation.
void begin_hook();

// Called by the script backends to check if the budget is blown. Returns true
// when the caller should abort.
bool over_budget();

// Number of Lua bytecode instructions between hook checks. Larger = less
// overhead, smaller = tighter timeout precision. Default 1000.
void set_lua_instruction_step(int n);
int  get_lua_instruction_step();

} // namespace paris::sandbox
