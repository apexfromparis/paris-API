#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace paris::net {

struct HttpResponse {
    bool        ok = false;
    int         status = 0;
    std::string body;      // raw bytes
};

// Synchronous. Timeout in milliseconds; 0 or negative uses a 15 second default.
HttpResponse http_get (const std::string& url, int timeout_ms = 0);
HttpResponse http_post(const std::string& url, const std::string& content_type,
                       const std::string& body, int timeout_ms = 0);

// ---------- WebSocket ----------

enum class WsFrameType : int { Text = 0, Binary = 1, Closed = 2, None = 3 };

class WebSocket {
public:
    virtual ~WebSocket() = default;
    virtual bool         is_open() const = 0;
    virtual bool         send_text(const std::string& s) = 0;
    virtual bool         send_binary(const std::string& bytes) = 0;
    virtual bool         send_json(const std::string& already_encoded) = 0; // scripts encode via json::stringify first
    virtual bool         close(int code = 1000) = 0;

    // Blocking receive. Returns nullopt on close / error.
    virtual std::optional<std::pair<std::string, WsFrameType>> recv() = 0;
    // Non-blocking: returns None if nothing queued.
    virtual std::pair<std::string, WsFrameType>                poll() = 0;
};

struct WsResult {
    std::unique_ptr<WebSocket> ws;
    std::string                error; // populated when ws == nullptr
};

WsResult ws_connect(const std::string& url, int timeout_ms = 0);

} // namespace paris::net
