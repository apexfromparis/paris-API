// Windows-only implementation on top of WinHTTP. On non-Windows the module
// compiles into a stub that always returns failure — enough to keep the API
// surface intact while the port is done.

#include "paris/net.hpp"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winhttp.h>
    #pragma comment(lib, "winhttp.lib")
#endif

#include <atomic>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

namespace paris::net {

#ifdef _WIN32
namespace {

std::wstring to_wide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), int(s.size()), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), int(s.size()), w.data(), n);
    return w;
}

struct ParsedUrl {
    bool         secure = false;
    std::wstring host;
    INTERNET_PORT port = 0;
    std::wstring path;
};

std::optional<ParsedUrl> parse_url(const std::string& url) {
    auto wurl = to_wide(url);
    URL_COMPONENTS uc{};
    uc.dwStructSize      = sizeof(uc);
    wchar_t scheme[16]  {}, host[256]{}, path[1024]{};
    uc.lpszScheme        = scheme; uc.dwSchemeLength    = _countof(scheme);
    uc.lpszHostName      = host;   uc.dwHostNameLength  = _countof(host);
    uc.lpszUrlPath       = path;   uc.dwUrlPathLength   = _countof(path);
    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &uc)) return std::nullopt;
    ParsedUrl out;
    out.secure = (uc.nScheme == INTERNET_SCHEME_HTTPS || uc.nScheme == INTERNET_SCHEME_SOCKS);
    out.host   = host;
    out.port   = uc.nPort;
    out.path   = path[0] ? path : L"/";
    return out;
}

HttpResponse do_request(const std::string& url, const wchar_t* verb,
                        const std::string& content_type,
                        const std::string& body, int timeout_ms) {
    HttpResponse r;
    auto parsed = parse_url(url);
    if (!parsed) return r;

    HINTERNET session = WinHttpOpen(L"paris-scripting/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return r;

    int t = timeout_ms > 0 ? timeout_ms : 15000;
    WinHttpSetTimeouts(session, t, t, t, t);

    HINTERNET conn = WinHttpConnect(session, parsed->host.c_str(), parsed->port, 0);
    if (!conn) { WinHttpCloseHandle(session); return r; }

    DWORD flags = parsed->secure ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(conn, verb, parsed->path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) { WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return r; }

    std::wstring headers;
    if (!content_type.empty())
        headers = L"Content-Type: " + to_wide(content_type) + L"\r\n";

    BOOL sent = WinHttpSendRequest(req,
        headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(),
        headers.empty() ? 0 : DWORD(-1L),
        body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(),
        DWORD(body.size()), DWORD(body.size()), 0);

    if (sent) sent = WinHttpReceiveResponse(req, nullptr);

    if (sent) {
        DWORD code = 0; DWORD size = sizeof(code);
        WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &code, &size, WINHTTP_NO_HEADER_INDEX);
        r.status = int(code);

        DWORD avail = 0;
        while (WinHttpQueryDataAvailable(req, &avail) && avail) {
            std::string chunk(avail, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(req, chunk.data(), avail, &read)) break;
            chunk.resize(read);
            r.body += chunk;
        }
        r.ok = true;
    }

    WinHttpCloseHandle(req);
    WinHttpCloseHandle(conn);
    WinHttpCloseHandle(session);
    return r;
}

} // namespace

HttpResponse http_get(const std::string& url, int timeout_ms) {
    return do_request(url, L"GET", "", "", timeout_ms);
}

HttpResponse http_post(const std::string& url, const std::string& ct,
                       const std::string& body, int timeout_ms) {
    return do_request(url, L"POST", ct, body, timeout_ms);
}

// ---------- WebSocket (WinHTTP) ----------

namespace {

class WinHttpWebSocket final : public WebSocket {
public:
    WinHttpWebSocket(HINTERNET session, HINTERNET conn, HINTERNET ws)
        : session_(session), conn_(conn), ws_(ws) {}

    ~WinHttpWebSocket() override {
        close(1000);
        if (ws_)      WinHttpCloseHandle(ws_);
        if (conn_)    WinHttpCloseHandle(conn_);
        if (session_) WinHttpCloseHandle(session_);
    }

    bool is_open() const override { return ws_ != nullptr && open_.load(); }

    bool send_text(const std::string& s) override {
        if (!ws_ || !open_) return false;
        return WinHttpWebSocketSend(ws_,
            WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
            (PVOID)s.data(), DWORD(s.size())) == ERROR_SUCCESS;
    }
    bool send_binary(const std::string& b) override {
        if (!ws_ || !open_) return false;
        return WinHttpWebSocketSend(ws_,
            WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
            (PVOID)b.data(), DWORD(b.size())) == ERROR_SUCCESS;
    }
    bool send_json(const std::string& j) override { return send_text(j); }

    bool close(int code) override {
        if (!ws_ || !open_) return false;
        open_ = false;
        return WinHttpWebSocketClose(ws_, USHORT(code), nullptr, 0) == ERROR_SUCCESS;
    }

    std::optional<std::pair<std::string, WsFrameType>> recv() override {
        if (!ws_ || !open_) return std::nullopt;
        std::string out;
        char buffer[4096];
        while (true) {
            DWORD read = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
            DWORD err = WinHttpWebSocketReceive(ws_, buffer, sizeof(buffer), &read, &type);
            if (err != ERROR_SUCCESS) { open_ = false; return std::nullopt; }
            out.append(buffer, read);
            if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) {
                open_ = false;
                return std::make_pair(std::move(out), WsFrameType::Closed);
            }
            if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE)
                return std::make_pair(std::move(out), WsFrameType::Text);
            if (type == WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE)
                return std::make_pair(std::move(out), WsFrameType::Binary);
            // *_FRAGMENT types → keep looping to accumulate the whole message.
        }
    }

    std::pair<std::string, WsFrameType> poll() override {
        // WinHTTP has no non-blocking primitive; polling maps to a very short
        // timeout receive on a separate thread in production code. Here we
        // return None to keep the behaviour predictable — advanced users can
        // spin up their own background task and buffer frames.
        return { {}, WsFrameType::None };
    }

private:
    HINTERNET session_ = nullptr;
    HINTERNET conn_    = nullptr;
    HINTERNET ws_      = nullptr;
    std::atomic<bool> open_{ true };
};

} // namespace

WsResult ws_connect(const std::string& url, int timeout_ms) {
    WsResult r;
    auto parsed = parse_url(url);
    if (!parsed) { r.error = "invalid url"; return r; }

    HINTERNET session = WinHttpOpen(L"paris-scripting/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { r.error = "WinHttpOpen failed"; return r; }
    int t = timeout_ms > 0 ? timeout_ms : 15000;
    WinHttpSetTimeouts(session, t, t, t, t);

    HINTERNET conn = WinHttpConnect(session, parsed->host.c_str(), parsed->port, 0);
    if (!conn) { WinHttpCloseHandle(session); r.error = "connect failed"; return r; }

    DWORD flags = parsed->secure ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(conn, L"GET", parsed->path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) { WinHttpCloseHandle(conn); WinHttpCloseHandle(session); r.error = "openreq failed"; return r; }

    WinHttpSetOption(req, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0);
    if (!WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            nullptr, 0, 0, 0) ||
        !WinHttpReceiveResponse(req, nullptr)) {
        WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session);
        r.error = "handshake failed"; return r;
    }

    HINTERNET ws = WinHttpWebSocketCompleteUpgrade(req, 0);
    WinHttpCloseHandle(req);
    if (!ws) {
        WinHttpCloseHandle(conn); WinHttpCloseHandle(session);
        r.error = "upgrade failed"; return r;
    }

    r.ws = std::make_unique<WinHttpWebSocket>(session, conn, ws);
    return r;
}

#else // !_WIN32

HttpResponse http_get (const std::string&, int)                                     { return {}; }
HttpResponse http_post(const std::string&, const std::string&, const std::string&, int) { return {}; }
WsResult     ws_connect(const std::string&, int)                                    { WsResult r; r.error = "not implemented on this platform"; return r; }

#endif

} // namespace paris::net
