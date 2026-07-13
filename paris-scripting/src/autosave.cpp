#include "paris/autosave.hpp"

#include "paris/callbacks.hpp"
#include "paris/fs.hpp"
#include "paris/ui.hpp"

namespace paris::autosave {

bool save(const std::string& path) {
    return fs::create_file(path, ui::construct_config());
}

bool load(const std::string& path) {
    if (!fs::does_file_exist(path)) return false;
    auto r = fs::read_file(path);
    if (!r.ok) return false;
    ui::apply_config(r.data);
    return true;
}

void install_hooks(const std::string& path) {
    // Fire the load on the very next `on_frame` tick so scripts have had a
    // full frame to declare their UI. Track it with a flag so we only load
    // once.
    static bool loaded = false;
    loaded = false;

    callbacks::add("on_frame", [path]() {
        if (loaded) return;
        loaded = true;
        load(path);
    });

    callbacks::add("on_unload", [path]() {
        save(path);
    });
}

} // namespace paris::autosave
