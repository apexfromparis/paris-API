#include "paris/util.hpp"

#include <array>
#include <cctype>
#include <string>

namespace paris::util {

// ---------- base64 ----------

namespace {
    constexpr const char* kB64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::array<int, 256> make_b64_reverse() {
        std::array<int, 256> t{};
        t.fill(-1);
        for (int i = 0; i < 64; ++i) t[uint8_t(kB64[i])] = i;
        return t;
    }
}

std::string base64_encode(const std::string& in) {
    std::string out;
    out.reserve(((in.size() + 2) / 3) * 4);
    int  val = 0, valb = -6;
    for (uint8_t c : in) {
        val = (val << 8) | c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(kB64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(kB64[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::optional<std::string> base64_decode(const std::string& in) {
    static const auto rev = make_b64_reverse();
    std::string out;
    int val = 0, valb = -8;
    for (uint8_t c : in) {
        if (c == '=') break;
        int v = rev[c];
        if (v == -1) continue; // skip whitespace / stray chars
        val = (val << 6) | v;
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// ---------- hex ----------

std::string hex_encode(const std::string& in) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.resize(in.size() * 2);
    for (size_t i = 0; i < in.size(); ++i) {
        uint8_t c = uint8_t(in[i]);
        out[i*2]     = kHex[c >> 4];
        out[i*2 + 1] = kHex[c & 0xF];
    }
    return out;
}

std::optional<std::string> hex_decode(const std::string& in) {
    auto val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    if (in.size() % 2) return std::nullopt;
    std::string out;
    out.reserve(in.size() / 2);
    for (size_t i = 0; i < in.size(); i += 2) {
        int hi = val(in[i]), lo = val(in[i + 1]);
        if (hi < 0 || lo < 0) return std::nullopt;
        out.push_back(char((hi << 4) | lo));
    }
    return out;
}

// ---------- url ----------

namespace {
    bool url_unreserved(char c) {
        return std::isalnum(uint8_t(c)) || c == '-' || c == '_' || c == '.' || c == '~';
    }
}

std::string url_encode(const std::string& in) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    for (uint8_t c : in) {
        if (url_unreserved(char(c))) out.push_back(char(c));
        else {
            out.push_back('%');
            out.push_back(kHex[c >> 4]);
            out.push_back(kHex[c & 0xF]);
        }
    }
    return out;
}

std::string url_decode(const std::string& in) {
    auto val = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
            int hi = val(in[i + 1]), lo = val(in[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(char((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        out.push_back(in[i]);
    }
    return out;
}

// ---------- fnv ----------

uint64_t fnv1a64(const std::string& in) {
    uint64_t h = 14695981039346656037ULL;
    for (uint8_t c : in) { h ^= c; h *= 1099511628211ULL; }
    return h;
}

} // namespace paris::util
