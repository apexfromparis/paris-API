# net

Low-level networking. HTTP GET / POST and WebSocket clients. Windows only —
uses WinHTTP under the hood.

Not currently exposed in AngelScript.

## HTTP

Synchronous. Timeout is in milliseconds; `0` or omitted defaults to 15 s.

| Function                                                | Returns              |
|---------------------------------------------------------|----------------------|
| `net.http_get(url, timeout_ms?)`                        | `(ok, status, body)` |
| `net.http_post(url, content_type, body, timeout_ms?)`   | `(ok, status, body)` |

* `ok` — `true` if the request completed (any HTTP status).
* `status` — the HTTP status code (0 when `ok = false`).
* `body` — raw response bytes.

{% hint style="warning" %}
HTTP calls block the calling thread. Fire them from button callbacks or
coroutines, not from `on_frame`.
{% endhint %}

## WebSocket

`net.ws_connect(url, timeout_ms?)` returns `(ws, err)`. On success `ws` is the
handle; on failure `ws` is `nil` and `err` holds a description.

### Sending

| Method                | Purpose |
|-----------------------|---------|
| `ws:send_text(msg)`   | UTF-8 text frame. |
| `ws:send_binary(bytes)` | Binary frame. |
| `ws:send_json(str)`   | Convenience — send a pre-encoded JSON string as text. |

### Receiving

| Method     | Purpose |
|------------|---------|
| `ws:recv()`| Blocking. Returns `(message, type)` where type is `0` text, `1` binary, `2` closed, `3` none. |
| `ws:poll()`| Non-blocking. Returns the same shape; `type = 3` when nothing queued. |

### Management

| Method              | Purpose |
|---------------------|---------|
| `ws:is_open()`      | Connection state. |
| `ws:close(code?)`   | Graceful shutdown. Default code `1000`. |

## Examples

### JSON API request

```lua
panel:add_button("Fetch time", function()
    local ok, code, body = net.http_get(
        "https://worldtimeapi.org/api/timezone/Etc/UTC", 5000)
    if not ok or code ~= 200 then
        engine.log_error("http failed: " .. tostring(code))
        return
    end
    local parsed = json.decode(body)
    if parsed then
        engine.log("utc: " .. parsed.datetime)
    end
end)
```

### POST JSON

```lua
local payload = json.encode({ event = "boot", user = engine.get_user_name() })
local ok, code, resp = net.http_post(
    "https://example.com/telemetry",
    "application/json",
    payload,
    3000)
```
