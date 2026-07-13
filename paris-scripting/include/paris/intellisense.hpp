#pragma once

#include <functional>
#include <string>
#include <vector>

namespace paris::intellisense {

// One completion candidate. `label` is what the user sees, `insert` is what
// gets typed on accept, `detail` is the trailing right-side hint.
struct Completion {
    std::string label;
    std::string insert;
    std::string detail;
};

// Provider signatures. `line_text` is the current line up to the cursor,
// `col` is the 0-based cursor column. Return an empty vector to opt out.
using completion_fn_t = std::function<std::vector<Completion>(const std::string& file,
                                                              const std::string& line_text,
                                                              int col)>;

// Hover provider. `word` is the token under the cursor. Return "" to opt out.
using hover_fn_t      = std::function<std::string(const std::string& file,
                                                  const std::string& word,
                                                  int line)>;

// Register a provider tied to the current script — dropped on unload.
void register_completion(completion_fn_t fn);
void register_hover     (hover_fn_t fn);

// Host-side: query every provider and merge results.
std::vector<Completion> collect_completions(const std::string& file,
                                            const std::string& line_text,
                                            int col);
std::string             collect_hover      (const std::string& file,
                                            const std::string& word,
                                            int line);

// Called by the engine on unload — drops providers registered by `section`.
void drop_section(const std::string& section);

} // namespace paris::intellisense
