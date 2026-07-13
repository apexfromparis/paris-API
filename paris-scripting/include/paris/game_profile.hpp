#pragma once

#include "types.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Per-game backend. Your usermode (per title: CS2, CS:GO, or anything else)
// implements one of these and calls `install()`. Everything the paris-compat
// Lua shim needs at runtime routes through the profile — that's what makes
// the same script work across different games without rewrites.
//
// Any function left null returns a sensible zero; scripts still load and
// bindings still resolve, they just see "no data".

namespace paris::game {

// Handle to a player / entity — an opaque integer that the profile chooses
// how to interpret (memory address, index, whatever fits your backend).
using entity_id = uint64_t;

struct EntityInfo {
    bool     valid       = false;
    bool     alive       = false;
    bool     enemy       = false;
    bool     dormant     = false;
    int      team        = 0;
    int      health      = 0;
    int      armor       = 0;
    int      class_id    = 0;
    std::string name;
    vector3  origin;
    vector3  eye_pos;
    vector3  velocity;
    vector3  view_angles;
    entity_id active_weapon = 0;
};

struct WeaponInfo {
    bool     valid    = false;
    int      type     = 0; // 0 = knife, > 0 = other (game specific)
    int      ammo     = 0;
    std::string name;
};

struct NetChanInfo {
    bool  valid   = false;
    float latency = 0.f; // seconds, average of the two directions
    int   in_seq  = 0;
    int   out_seq = 0;
};

// User command / cmd_t. Populated by the host on every create_move tick, then
// mutated by scripts, then read back by the host to send to the server.
struct UserCmd {
    int     tick_count       = 0;
    vector3 view_angles      = {};
    float   forward_move     = 0.f;
    float   side_move        = 0.f;
    float   up_move          = 0.f;
    int     buttons          = 0;
    int     impulse          = 0;
    int     weapon_select    = 0;
    int     weapon_subtype   = 0;
    int     random_seed      = 0;
    int     mouse_dx         = 0;
    int     mouse_dy         = 0;
    bool    send_packet      = true;
};

// The pluggable backend.
struct Profile {
    std::string name;             // "cs2", "csgo", "custom", …

    // World / engine state
    std::function<bool()>                          is_in_game;
    std::function<NetChanInfo()>                   get_netchan;
    std::function<float()>                         get_real_time;
    std::function<float()>                         get_curtime;
    std::function<int()>                           get_tick;
    std::function<vector2()>                       get_screen_size;

    // Entities
    std::function<entity_id()>                     get_local_pawn;
    std::function<entity_id()>                     get_local_controller;
    std::function<std::vector<entity_id>()>        get_players;
    std::function<std::vector<entity_id>()>        get_controllers;
    std::function<EntityInfo(entity_id)>           inspect_entity;
    std::function<WeaponInfo(entity_id)>           inspect_weapon;

    // create_move / usercmd. The host fills a fresh UserCmd, calls
    // fire_create_move, then reads the (possibly mutated) command back.
    std::function<UserCmd()>                       current_cmd;
    std::function<void(const UserCmd&)>            apply_cmd;
};

// Install the currently active profile. Passing an empty profile disables
// game-side queries — scripts still run against the socket, they just get
// zeroed data.
void install(Profile p);

// Query the current profile.
const Profile& current();
bool           has_profile();

// Convenience wrappers — these are the ones the paris-compat Lua shim calls
// into. They forward to `current().<fn>()` with fallbacks.
bool                     is_in_game();
NetChanInfo              get_netchan();
float                    get_real_time();
float                    get_curtime();
int                      get_tick();
vector2                  get_screen_size();
entity_id                get_local_pawn();
entity_id                get_local_controller();
std::vector<entity_id>   get_players();
std::vector<entity_id>   get_controllers();
EntityInfo               inspect_entity(entity_id id);
WeaponInfo               inspect_weapon(entity_id id);

// create_move dispatch — the host calls this each tick before sending.
// Callbacks named "create_move" fire; the cmd carried inside them is the one
// scripts mutate.
void fire_create_move(UserCmd& cmd);
UserCmd& current_cmd_ref(); // for the script bindings

// Helpers to auto-select a profile by executable name. Returns "" if no
// registered profile matches.
void        register_profile_matcher(std::string exe_substring, std::string profile_name);
std::string autodetect_profile(const std::string& process_name);

} // namespace paris::game
