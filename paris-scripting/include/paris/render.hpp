#pragma once

#include "types.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace paris::render {

// Text effect. Matches the perception.cx `draw_text` fifth argument.
enum class TextEffect : int {
    None    = 0,
    Outline = 1,
    Shadow  = 2,
    Glow    = 3,
};

// Handle for a texture created via `create_bitmap`. The host owns the resource;
// scripts hold this integer id.
using bitmap_t = uint32_t;

struct TextSize { float w = 0.f; float h = 0.f; };

// ---------- Viewport / metrics ----------

vector2 get_view();           // width, height in overlay pixels
float   get_view_scale();     // DPI scale (1.0 = 100%)
float   get_fps();

// ---------- Shapes ----------

void draw_rect        (const vector2& min, const vector2& max, const color_t& c, float thickness = 1.f, float rounding = 0.f);
void draw_rect_filled (const vector2& min, const vector2& max, const color_t& c, float rounding = 0.f);
void draw_gradient    (const vector2& min, const vector2& max,
                       const color_t& tl, const color_t& tr,
                       const color_t& br, const color_t& bl);

void draw_line        (const vector2& a, const vector2& b, const color_t& c, float thickness = 1.f);
void draw_circle      (const vector2& center, float radius, const color_t& c, float thickness = 1.f, int segments = 24);
void draw_circle_filled(const vector2& center, float radius, const color_t& c, int segments = 24);
void draw_arc         (const vector2& center, float radius, float from_rad, float to_rad,
                       const color_t& c, float thickness = 1.f, int segments = 24);
void draw_triangle    (const vector2& a, const vector2& b, const vector2& d, const color_t& c, float thickness = 1.f);
void draw_triangle_filled(const vector2& a, const vector2& b, const vector2& d, const color_t& c);
void draw_polygon     (const std::vector<vector2>& pts, const color_t& c, float thickness = 1.f);
void draw_polygon_filled(const std::vector<vector2>& pts, const color_t& c);

// ---------- Text ----------

// Font size — perception supports 18/20/24/28 out of the box. The host maps
// these to preloaded fonts. Custom fonts loaded via `load_font` return a fresh
// id which you can pass here instead.
using font_id_t = int;
constexpr font_id_t kFont18 = 18;
constexpr font_id_t kFont20 = 20;
constexpr font_id_t kFont24 = 24;
constexpr font_id_t kFont28 = 28;

void     draw_text     (const vector2& pos, const color_t& c, const std::string& text,
                        font_id_t font = kFont20, TextEffect effect = TextEffect::None,
                        const color_t& effect_color = { 0, 0, 0, 200 });
TextSize get_text_size (const std::string& text, font_id_t font = kFont20);
float    get_char_advance(uint32_t codepoint, font_id_t font = kFont20);

// Load a custom TTF from a byte buffer. Returns an id you can pass to draw_text.
font_id_t load_font_from_memory(const std::string& ttf_bytes, float pixel_size);
font_id_t load_font_from_file  (const std::string& path,       float pixel_size);

// ---------- Bitmaps ----------

// Create a bitmap from raw RGBA8 bytes (row-major, top-left origin).
bitmap_t create_bitmap(const std::string& rgba8, int width, int height);
void     destroy_bitmap(bitmap_t id);
void     draw_bitmap  (bitmap_t id, const vector2& min, const vector2& max,
                       const color_t& tint = { 255, 255, 255, 255 }, float rounding = 0.f);

// ---------- Clipping ----------

void clip_push(const vector2& min, const vector2& max);
void clip_pop();

// ---------- Backend wiring ----------

// The host renderer plugs concrete implementations in here. Anything left null
// is a no-op — a script can draw without crashing even if some functions aren't
// available.
struct Backend {
    // Metrics
    std::function<vector2()>                                     get_view;
    std::function<float()>                                       get_view_scale;
    std::function<float()>                                       get_fps;

    // Shapes
    std::function<void(vector2, vector2, color_t, float, float)> draw_rect;
    std::function<void(vector2, vector2, color_t, float)>        draw_rect_filled;
    std::function<void(vector2, vector2, color_t, color_t, color_t, color_t)> draw_gradient;
    std::function<void(vector2, vector2, color_t, float)>        draw_line;
    std::function<void(vector2, float, color_t, float, int)>     draw_circle;
    std::function<void(vector2, float, color_t, int)>            draw_circle_filled;
    std::function<void(vector2, float, float, float, color_t, float, int)> draw_arc;
    std::function<void(vector2, vector2, vector2, color_t, float)> draw_triangle;
    std::function<void(vector2, vector2, vector2, color_t)>      draw_triangle_filled;
    std::function<void(std::vector<vector2>, color_t, float)>    draw_polygon;
    std::function<void(std::vector<vector2>, color_t)>           draw_polygon_filled;

    // Text
    std::function<void(vector2, color_t, std::string, font_id_t, TextEffect, color_t)> draw_text;
    std::function<TextSize(std::string, font_id_t)>              get_text_size;
    std::function<float(uint32_t, font_id_t)>                    get_char_advance;
    std::function<font_id_t(std::string, float)>                 load_font_from_memory;
    std::function<font_id_t(std::string, float)>                 load_font_from_file;

    // Bitmaps
    std::function<bitmap_t(std::string, int, int)>               create_bitmap;
    std::function<void(bitmap_t)>                                destroy_bitmap;
    std::function<void(bitmap_t, vector2, vector2, color_t, float)> draw_bitmap;

    // Clipping
    std::function<void(vector2, vector2)>                        clip_push;
    std::function<void()>                                        clip_pop;

    // Projection
    std::function<bool(vector3, vector2&)>                       world_to_screen;
};

void install(Backend b);

// Returns false when the backend is missing or the point is behind the camera.
bool world_to_screen(const vector3& w, vector2& out);

} // namespace paris::render
