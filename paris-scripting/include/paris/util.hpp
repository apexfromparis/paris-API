#pragma once

#include <optional>
#include <string>

// Byte-safe encoders / decoders — none of these interpret the input as UTF-8.
// A failed decode returns nullopt so the script can surface an error rather
// than get a corrupt result.

namespace paris::util {

std::string                base64_encode(const std::string& bytes);
std::optional<std::string> base64_decode(const std::string& text);

// Uppercase hex on encode; decode accepts either case.
std::string                hex_encode(const std::string& bytes);
std::optional<std::string> hex_decode(const std::string& text);

// Percent-encoding per RFC 3986 unreserved set. `+` is NOT translated to space
// on decode — spaces have to be `%20`. That matches perception.cx and avoids
// surprises with form-encoded payloads that need the raw `+` preserved.
std::string url_encode(const std::string& bytes);
std::string url_decode(const std::string& text);

// A cheap non-cryptographic hash (FNV-1a 64-bit). Fine for map keys, IDs, and
// change detection. Do NOT use for anything security-sensitive.
uint64_t fnv1a64(const std::string& bytes);

} // namespace paris::util
