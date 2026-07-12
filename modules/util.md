# util

Byte-safe encoders / decoders. Every function operates on raw bytes, not
UTF-8 text — safe for binary payloads and null bytes. Decoders return `nil`
on failure.

Not currently exposed in AngelScript.

## Functions

| Function                     | Returns  | Purpose |
|------------------------------|----------|---------|
| `util.base64_encode(bytes)`  | `string` | Standard base64. |
| `util.base64_decode(text)`   | `string?`| Decoder — `nil` on failure. |
| `util.hex_encode(bytes)`     | `string` | Uppercase hex. |
| `util.hex_decode(text)`      | `string?`| Case-insensitive. `nil` on failure. |
| `util.url_encode(bytes)`     | `string` | Percent-encode per RFC 3986. |
| `util.url_decode(text)`      | `string` | Percent-decode. `+` is **not** translated to space. |
| `util.fnv1a64(bytes)`        | `number` | 64-bit FNV-1a hash. Non-cryptographic. |

## Examples

```lua
-- pack binary blob for a URL param
local raw = fs.read_file("data.bin")
local param = util.url_encode(util.base64_encode(raw))

-- verify with a hash
local h = util.fnv1a64("hello world")
engine.log(string.format("hash = 0x%x", h))
```
