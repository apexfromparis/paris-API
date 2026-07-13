#pragma once

#include <string>
#include <vector>

namespace paris::tools {

// AI tool declared by an extension. Roughly matches the OpenAI / Anthropic
// tool schema — `params_json` is the JSON Schema describing the parameters.
struct ToolParam {
    std::string name;
    std::string type;     // "string" | "number" | "boolean" | "object" | "array"
    std::string desc;
    bool        required = false;
};

struct Tool {
    std::string             name;
    std::string             description;
    std::string             params_json; // pre-built JSON Schema, if the extension supplies one
    std::vector<ToolParam>  params;
    std::string             section;     // owning script — used for auto-cleanup
};

// Registration. `params_json` can be a bare JSON Schema; if it's empty the
// tool is built up incrementally via `register_param`.
void register_tool (const std::string& name, const std::string& description,
                    const std::string& params_json = "");
void register_param(const std::string& tool, const std::string& name,
                    const std::string& type, const std::string& description,
                    bool required = false);
void unregister    (const std::string& name);

// Enumerate every registered tool. The host serialises this into the outgoing
// tool schema when calling the model.
std::vector<Tool> list();

// Called by the engine on unload — drops tools registered by `section`.
void drop_section(const std::string& section);

} // namespace paris::tools
