#include "paris/render.hpp"

#include <mutex>

// This file is intentionally a pile of thin dispatchers — every function looks
// up the backend under a lock and forwards. That gives you one place to swap
// the concrete renderer (ImGui, D3D, whatever) without touching the scripts.

namespace paris::render {

namespace {
    std::mutex g_mtx;
    Backend    g_backend;
}

void install(Backend b) {
    std::lock_guard lk(g_mtx);
    g_backend = std::move(b);
}

vector2 get_view() {
    std::lock_guard lk(g_mtx);
    return g_backend.get_view ? g_backend.get_view() : vector2{};
}
float get_view_scale() {
    std::lock_guard lk(g_mtx);
    return g_backend.get_view_scale ? g_backend.get_view_scale() : 1.f;
}
float get_fps() {
    std::lock_guard lk(g_mtx);
    return g_backend.get_fps ? g_backend.get_fps() : 0.f;
}

void draw_rect(const vector2& mn, const vector2& mx, const color_t& c, float t, float r) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_rect) g_backend.draw_rect(mn, mx, c, t, r);
}
void draw_rect_filled(const vector2& mn, const vector2& mx, const color_t& c, float r) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_rect_filled) g_backend.draw_rect_filled(mn, mx, c, r);
}
void draw_gradient(const vector2& mn, const vector2& mx,
                   const color_t& tl, const color_t& tr,
                   const color_t& br, const color_t& bl) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_gradient) g_backend.draw_gradient(mn, mx, tl, tr, br, bl);
}
void draw_line(const vector2& a, const vector2& b, const color_t& c, float t) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_line) g_backend.draw_line(a, b, c, t);
}
void draw_circle(const vector2& c, float r, const color_t& col, float t, int seg) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_circle) g_backend.draw_circle(c, r, col, t, seg);
}
void draw_circle_filled(const vector2& c, float r, const color_t& col, int seg) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_circle_filled) g_backend.draw_circle_filled(c, r, col, seg);
}
void draw_arc(const vector2& c, float r, float f, float to, const color_t& col, float t, int seg) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_arc) g_backend.draw_arc(c, r, f, to, col, t, seg);
}
void draw_triangle(const vector2& a, const vector2& b, const vector2& d, const color_t& c, float t) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_triangle) g_backend.draw_triangle(a, b, d, c, t);
}
void draw_triangle_filled(const vector2& a, const vector2& b, const vector2& d, const color_t& c) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_triangle_filled) g_backend.draw_triangle_filled(a, b, d, c);
}
void draw_polygon(const std::vector<vector2>& pts, const color_t& c, float t) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_polygon) g_backend.draw_polygon(pts, c, t);
}
void draw_polygon_filled(const std::vector<vector2>& pts, const color_t& c) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_polygon_filled) g_backend.draw_polygon_filled(pts, c);
}

void draw_text(const vector2& pos, const color_t& col, const std::string& s,
               font_id_t font, TextEffect fx, const color_t& fx_col) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_text) g_backend.draw_text(pos, col, s, font, fx, fx_col);
}
TextSize get_text_size(const std::string& s, font_id_t font) {
    std::lock_guard lk(g_mtx);
    return g_backend.get_text_size ? g_backend.get_text_size(s, font) : TextSize{};
}
float get_char_advance(uint32_t cp, font_id_t font) {
    std::lock_guard lk(g_mtx);
    return g_backend.get_char_advance ? g_backend.get_char_advance(cp, font) : 0.f;
}
font_id_t load_font_from_memory(const std::string& bytes, float px) {
    std::lock_guard lk(g_mtx);
    return g_backend.load_font_from_memory ? g_backend.load_font_from_memory(bytes, px) : -1;
}
font_id_t load_font_from_file(const std::string& path, float px) {
    std::lock_guard lk(g_mtx);
    return g_backend.load_font_from_file ? g_backend.load_font_from_file(path, px) : -1;
}

bitmap_t create_bitmap(const std::string& rgba8, int w, int h) {
    std::lock_guard lk(g_mtx);
    return g_backend.create_bitmap ? g_backend.create_bitmap(rgba8, w, h) : bitmap_t{};
}
void destroy_bitmap(bitmap_t id) {
    std::lock_guard lk(g_mtx);
    if (g_backend.destroy_bitmap) g_backend.destroy_bitmap(id);
}
void draw_bitmap(bitmap_t id, const vector2& mn, const vector2& mx, const color_t& tint, float r) {
    std::lock_guard lk(g_mtx);
    if (g_backend.draw_bitmap) g_backend.draw_bitmap(id, mn, mx, tint, r);
}

void clip_push(const vector2& mn, const vector2& mx) {
    std::lock_guard lk(g_mtx);
    if (g_backend.clip_push) g_backend.clip_push(mn, mx);
}
void clip_pop() {
    std::lock_guard lk(g_mtx);
    if (g_backend.clip_pop) g_backend.clip_pop();
}

bool world_to_screen(const vector3& w, vector2& out) {
    std::lock_guard lk(g_mtx);
    if (!g_backend.world_to_screen) return false;
    return g_backend.world_to_screen(w, out);
}

} // namespace paris::render

namespace paris {
bool vector3::to_screen(vector2& out) const { return render::world_to_screen(*this, out); }
} // namespace paris
