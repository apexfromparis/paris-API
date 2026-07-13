#include "paris/game_profile.hpp"

#include "paris/callbacks.hpp"

#include <any>
#include <mutex>
#include <unordered_map>

namespace paris::game {

namespace {
    std::mutex g_mtx;
    Profile    g_profile;

    // create_move state — filled by the host, mutated by callbacks, read back.
    UserCmd    g_current_cmd;

    // Registered exe matchers → profile names.
    std::vector<std::pair<std::string, std::string>> g_matchers;
}

void install(Profile p) {
    std::lock_guard lk(g_mtx);
    g_profile = std::move(p);
}

const Profile& current()      { std::lock_guard lk(g_mtx); return g_profile; }
bool           has_profile()  { std::lock_guard lk(g_mtx); return !g_profile.name.empty(); }

bool is_in_game() {
    std::lock_guard lk(g_mtx);
    return g_profile.is_in_game ? g_profile.is_in_game() : false;
}

NetChanInfo get_netchan() {
    std::lock_guard lk(g_mtx);
    return g_profile.get_netchan ? g_profile.get_netchan() : NetChanInfo{};
}

float   get_real_time()      { std::lock_guard lk(g_mtx); return g_profile.get_real_time    ? g_profile.get_real_time()    : 0.f; }
float   get_curtime()        { std::lock_guard lk(g_mtx); return g_profile.get_curtime      ? g_profile.get_curtime()      : 0.f; }
int     get_tick()           { std::lock_guard lk(g_mtx); return g_profile.get_tick         ? g_profile.get_tick()         : 0; }
vector2 get_screen_size()    { std::lock_guard lk(g_mtx); return g_profile.get_screen_size  ? g_profile.get_screen_size()  : vector2{}; }

entity_id get_local_pawn()       { std::lock_guard lk(g_mtx); return g_profile.get_local_pawn       ? g_profile.get_local_pawn()       : 0; }
entity_id get_local_controller() { std::lock_guard lk(g_mtx); return g_profile.get_local_controller ? g_profile.get_local_controller() : 0; }

std::vector<entity_id> get_players() {
    std::lock_guard lk(g_mtx);
    return g_profile.get_players ? g_profile.get_players() : std::vector<entity_id>{};
}
std::vector<entity_id> get_controllers() {
    std::lock_guard lk(g_mtx);
    return g_profile.get_controllers ? g_profile.get_controllers() : std::vector<entity_id>{};
}
EntityInfo inspect_entity(entity_id id) {
    std::lock_guard lk(g_mtx);
    return g_profile.inspect_entity ? g_profile.inspect_entity(id) : EntityInfo{};
}
WeaponInfo inspect_weapon(entity_id id) {
    std::lock_guard lk(g_mtx);
    return g_profile.inspect_weapon ? g_profile.inspect_weapon(id) : WeaponInfo{};
}

void fire_create_move(UserCmd& cmd) {
    { std::lock_guard lk(g_mtx); g_current_cmd = cmd; }
    // Bare event first, then payload event carrying the cmd — extensions can
    // hook whichever they prefer.
    callbacks::fire("create_move");
    callbacks::fire_any("create_move", std::any{ std::ref(g_current_cmd) });
    { std::lock_guard lk(g_mtx); cmd = g_current_cmd; }

    Profile p;
    { std::lock_guard lk(g_mtx); p = g_profile; }
    if (p.apply_cmd) p.apply_cmd(cmd);
}

UserCmd& current_cmd_ref() { return g_current_cmd; }

void register_profile_matcher(std::string exe, std::string profile_name) {
    std::lock_guard lk(g_mtx);
    g_matchers.push_back({ std::move(exe), std::move(profile_name) });
}

std::string autodetect_profile(const std::string& process_name) {
    std::lock_guard lk(g_mtx);
    for (auto& [needle, prof] : g_matchers) {
        if (process_name.find(needle) != std::string::npos) return prof;
    }
    return {};
}

} // namespace paris::game
