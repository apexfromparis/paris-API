#pragma once

#include <string>
#include <vector>

namespace paris::fs {

// Every path is treated as relative and reprojected into a fixed sandbox
// directory. Absolute paths and `..` traversal are rejected. The default root
// is Windows' `%USERPROFILE%\Documents\My Games\paris` — call `set_root` at
// boot if you want it elsewhere.
void        set_root(const std::string& absolute);
std::string get_root();

struct ReadResult {
    bool        ok = false;
    std::string data;
};

struct QueryEntry {
    std::string name;
    bool        is_directory = false;
    uint64_t    size = 0;
};

bool                    create_directory(const std::string& rel);
bool                    create_file     (const std::string& rel, const std::string& data);
bool                    does_file_exist (const std::string& rel);
ReadResult              read_file       (const std::string& rel);
bool                    delete_file     (const std::string& rel);
bool                    delete_directory(const std::string& rel);
std::vector<QueryEntry> query_directory (const std::string& rel,
                                         bool include_dirs = true,
                                         bool include_files = true,
                                         const std::vector<std::string>& extensions = {});

} // namespace paris::fs
