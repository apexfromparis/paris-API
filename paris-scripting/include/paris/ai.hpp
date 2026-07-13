#pragma once

#include <functional>
#include <string>
#include <vector>

namespace paris::ai {

// Message role in a chat conversation.
enum class Role { User, Assistant, System, Tool };

// One turn in the current conversation. `content` is the text; role is set
// via the enum.
struct Message {
    Role        role = Role::User;
    std::string content;
};

// ---------- Pipeline hooks — scripts register these to intercept AI activity ----------

// Fires just before a prompt is sent to the model. Return `false` to cancel
// the send entirely. Both parameters are mutable — mutate to rewrite the
// outbound prompt / system prompt.
using before_send_fn_t   = std::function<bool(std::string& prompt, std::string& system_prompt)>;

// Fires after the model returns. `response` is mutable — mutate to rewrite
// the visible reply.
using after_response_fn_t = std::function<void(std::string& response)>;

// Fires when the model calls a registered tool. Return the tool's result
// as a string. Return "" to fall through to the next handler.
using tool_call_fn_t     = std::function<std::string(const std::string& name,
                                                     const std::string& args_json)>;

// Fires after any tool call resolves, for observability.
using after_tool_fn_t    = std::function<void(const std::string& name,
                                              const std::string& args_json,
                                              const std::string& result)>;

// Fires when building the system prompt — return additional text to inject.
using system_inject_fn_t = std::function<std::string()>;

void on_before_send    (before_send_fn_t fn);
void on_after_response (after_response_fn_t fn);
void on_tool_call      (tool_call_fn_t fn);
void on_after_tool     (after_tool_fn_t fn);
void on_system_inject  (system_inject_fn_t fn);

// ---------- Host-side dispatch ----------

// Called by the host right before sending a prompt. Returns false if any
// hook vetoed; mutations to the strings persist across handlers.
bool fire_before_send   (std::string& prompt, std::string& system_prompt);
void fire_after_response(std::string& response);
std::string fire_tool_call (const std::string& name, const std::string& args_json);
void fire_after_tool    (const std::string& name, const std::string& args_json,
                         const std::string& result);
std::string fire_system_inject();

// ---------- Introspection ----------

std::string get_active_model();
void        set_active_model(const std::string& model);

int  get_chat_message_count();
Message get_chat_message(int index);
void    set_chat_history(std::vector<Message> history);

// Called by the engine on script unload — drops hooks tagged with `section`.
void drop_section(const std::string& section);

} // namespace paris::ai
