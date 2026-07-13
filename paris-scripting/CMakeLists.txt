cmake_minimum_required(VERSION 3.20)
project(paris_scripting CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Recommended workflow — vcpkg in manifest mode:
#   1. clone vcpkg somewhere and bootstrap it
#   2. cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
# vcpkg reads dependencies from vcpkg.json in the source tree.

find_package(sol2         CONFIG REQUIRED)
find_package(Lua          REQUIRED)
find_package(Angelscript  CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(paris_scripting STATIC
    src/log.cpp
    src/callbacks.cpp
    src/ui.cpp
    src/render.cpp
    src/input.cpp
    src/fs.cpp
    src/util.cpp
    src/json.cpp
    src/net.cpp
    src/engine.cpp
    src/sandbox.cpp
    src/hot_reload.cpp
    src/keybind_driver.cpp
    src/metadata.cpp
    src/autosave.cpp
    src/editor.cpp
    src/clipboard.cpp
    src/intellisense.cpp
    src/ai.cpp
    src/tools.cpp
    src/settings.cpp
    src/terminal.cpp
    src/script_lua.cpp
    src/script_as.cpp
)

target_include_directories(paris_scripting
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${LUA_INCLUDE_DIR})

target_link_libraries(paris_scripting
    PUBLIC  sol2::sol2
            ${LUA_LIBRARIES}
            Angelscript::angelscript
            nlohmann_json::nlohmann_json)

if (WIN32)
    target_link_libraries(paris_scripting PUBLIC winhttp shell32 ole32)
endif()

if (MSVC)
    target_compile_options(paris_scripting PRIVATE /W4 /permissive- /Zc:__cplusplus)
    target_compile_definitions(paris_scripting PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    target_compile_options(paris_scripting PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(paris_demo demo/main.cpp)
target_link_libraries(paris_demo PRIVATE paris_scripting)
