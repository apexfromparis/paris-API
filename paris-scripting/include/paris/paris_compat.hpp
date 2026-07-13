#pragma once

// paris/gamesense compatibility shim. Installs Lua tables (`gui`, `events`,
// `draw`, `game`, `entities`, `hvh`, `theme`, `mods`, `ant`, `utils`, `ffi`)
// on top of the paris scripting core so leaked / third-party scripts that
// target that API surface can execute as-is.
//
// Not every function on those namespaces resolves to real behaviour — where
// no game_profile backend is wired up, entity queries return empty, ffi
// calls no-op, etc. That means scripts *load and register their menu*, even
// on an unbacked machine; the runtime portions light up as soon as your
// game_profile fills in.

namespace sol { class state; }

namespace paris::paris_compat {

// Installs the shim into the supplied Lua state. Safe to call multiple times
// — later calls are no-ops.
void install(sol::state& lua);

} // namespace paris::paris_compat
