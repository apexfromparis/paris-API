#pragma once

// Optional single-header ImGui backend for paris. Requires ImGui in your
// project. This file draws BOTH the paris menu (via ui::layout()) AND wires
// the render backend so drawing calls from scripts land on ImGui's foreground
// draw list.
//
// Usage:
//     #include "paris/imgui_backend.hpp"
//     paris::imgui_backend::install();   // once, after ImGui context creation
//     paris::imgui_backend::draw_menu(); // once per frame, inside your ImGui frame
//
// Anything you don't like is trivial to override — copy this header into your
// project and edit. Nothing here is required by the core engine.

#include "input.hpp"
#include "render.hpp"
#include "types.hpp"
#include "ui.hpp"

#include <imgui.h>

#include <cstdio>
#include <string>
#include <vector>

namespace paris::imgui_backend {

inline ImU32 imcol(const color_t& c) {
    return IM_COL32(c.r, c.g, c.b, c.a);
}

inline ImVec2 imvec(const vector2& v) { return { v.x, v.y }; }

inline void install() {
    render::Backend b;

    b.get_view = []() {
        auto& io = ImGui::GetIO();
        return vector2{ io.DisplaySize.x, io.DisplaySize.y };
    };
    b.get_view_scale = []() { return ImGui::GetIO().DisplayFramebufferScale.x; };
    b.get_fps        = []() { return ImGui::GetIO().Framerate; };

    b.draw_rect = [](vector2 mn, vector2 mx, color_t c, float t, float r) {
        ImGui::GetForegroundDrawList()->AddRect(imvec(mn), imvec(mx), imcol(c), r, 0, t);
    };
    b.draw_rect_filled = [](vector2 mn, vector2 mx, color_t c, float r) {
        ImGui::GetForegroundDrawList()->AddRectFilled(imvec(mn), imvec(mx), imcol(c), r);
    };
    b.draw_gradient = [](vector2 mn, vector2 mx, color_t tl, color_t tr, color_t br, color_t bl) {
        ImGui::GetForegroundDrawList()->AddRectFilledMultiColor(
            imvec(mn), imvec(mx), imcol(tl), imcol(tr), imcol(br), imcol(bl));
    };
    b.draw_line = [](vector2 a, vector2 b_, color_t c, float t) {
        ImGui::GetForegroundDrawList()->AddLine(imvec(a), imvec(b_), imcol(c), t);
    };
    b.draw_circle = [](vector2 c, float r, color_t col, float t, int seg) {
        ImGui::GetForegroundDrawList()->AddCircle(imvec(c), r, imcol(col), seg, t);
    };
    b.draw_circle_filled = [](vector2 c, float r, color_t col, int seg) {
        ImGui::GetForegroundDrawList()->AddCircleFilled(imvec(c), r, imcol(col), seg);
    };
    b.draw_triangle = [](vector2 a, vector2 b_, vector2 d, color_t c, float t) {
        ImGui::GetForegroundDrawList()->AddTriangle(imvec(a), imvec(b_), imvec(d), imcol(c), t);
    };
    b.draw_triangle_filled = [](vector2 a, vector2 b_, vector2 d, color_t c) {
        ImGui::GetForegroundDrawList()->AddTriangleFilled(imvec(a), imvec(b_), imvec(d), imcol(c));
    };
    b.draw_polygon = [](std::vector<vector2> pts, color_t c, float t) {
        std::vector<ImVec2> im; im.reserve(pts.size());
        for (auto& p : pts) im.push_back(imvec(p));
        ImGui::GetForegroundDrawList()->AddPolyline(im.data(), int(im.size()), imcol(c), ImDrawFlags_Closed, t);
    };
    b.draw_polygon_filled = [](std::vector<vector2> pts, color_t c) {
        std::vector<ImVec2> im; im.reserve(pts.size());
        for (auto& p : pts) im.push_back(imvec(p));
        ImGui::GetForegroundDrawList()->AddConvexPolyFilled(im.data(), int(im.size()), imcol(c));
    };

    b.draw_text = [](vector2 p, color_t c, std::string s,
                    render::font_id_t /*font*/, render::TextEffect fx, color_t fxc) {
        auto* dl = ImGui::GetForegroundDrawList();
        // Cheap outline / shadow / glow: draw once offset in the effect color,
        // then the main text on top. Font swapping requires custom font
        // registration on the ImGui side — the id-based API is a stub here.
        auto draw_offset = [&](float ox, float oy) {
            dl->AddText({ p.x + ox, p.y + oy }, imcol(fxc), s.c_str());
        };
        switch (fx) {
            case render::TextEffect::None: break;
            case render::TextEffect::Shadow:  draw_offset(1, 1); break;
            case render::TextEffect::Outline:
                draw_offset(-1, 0); draw_offset(1, 0);
                draw_offset(0, -1); draw_offset(0, 1);
                break;
            case render::TextEffect::Glow:
                for (int i = 1; i <= 3; ++i) {
                    draw_offset(-i, 0); draw_offset(i, 0);
                    draw_offset(0, -i); draw_offset(0, i);
                }
                break;
        }
        dl->AddText(imvec(p), imcol(c), s.c_str());
    };

    b.get_text_size = [](std::string s, render::font_id_t) {
        auto sz = ImGui::CalcTextSize(s.c_str());
        return render::TextSize{ sz.x, sz.y };
    };

    b.clip_push = [](vector2 mn, vector2 mx) {
        ImGui::GetForegroundDrawList()->PushClipRect(imvec(mn), imvec(mx), true);
    };
    b.clip_pop = []() { ImGui::GetForegroundDrawList()->PopClipRect(); };

    render::install(std::move(b));
}

// Push a paris FrameSnapshot from ImGui's IO for the current frame.
inline void snapshot_input() {
    input::FrameSnapshot snap;
    auto& io = ImGui::GetIO();
    snap.mouse_pos     = { io.MousePos.x, io.MousePos.y };
    snap.mouse_desktop = { io.MousePos.x, io.MousePos.y };
    snap.mouse_delta   = { io.MouseDelta.x, io.MouseDelta.y };
    snap.wheel         = io.MouseWheel;
    for (int vk = 0; vk < 256; ++vk) {
        bool held = ImGui::IsKeyDown(ImGuiKey(vk));
        snap.keys_down[vk]     = held;
        snap.keys_pressed[vk]  = ImGui::IsKeyPressed(ImGuiKey(vk), false);
        snap.keys_released[vk] = ImGui::IsKeyReleased(ImGuiKey(vk));
    }
    input::begin_frame(snap);
}

// Draws the paris menu into an ImGui window. Call once per frame between
// ImGui::NewFrame and ImGui::Render.
inline void draw_menu(const char* title = "paris") {
    if (!ImGui::Begin(title)) { ImGui::End(); return; }

    ui::set_menu_visible(true);
    auto tabs = ui::layout();

    if (ImGui::BeginTabBar("root")) {
        for (auto& tab : tabs) {
            std::string tab_name = "Tab " + std::to_string(tab.index);
            if (!ImGui::BeginTabItem(tab_name.c_str())) continue;

            if (ImGui::BeginTabBar("subtabs")) {
                for (auto& sub : tab.subtabs) {
                    if (!sub->active) continue;
                    if (!ImGui::BeginTabItem(sub->name.c_str())) continue;

                    for (auto& panel : sub->panels) {
                        if (ImGui::CollapsingHeader(panel->name.c_str(),
                                ImGuiTreeNodeFlags_DefaultOpen)) {
                            for (auto& el : panel->elements) {
                                if (!el->active) continue;
                                using namespace paris::ui;

                                switch (el->kind) {
                                case ElementKind::Checkbox: {
                                    bool v = std::get<bool>(el->value);
                                    if (ImGui::Checkbox(el->name.c_str(), &v)) el->value = v;
                                    for (auto& child : el->children) {
                                        if (child->kind == ElementKind::ColorPicker) {
                                            auto c = std::get<color_t>(child->value);
                                            float col[4] = { c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f };
                                            if (ImGui::ColorEdit4((child->name + "##" + el->name).c_str(), col))
                                                child->value = color_t{ int(col[0]*255), int(col[1]*255),
                                                                        int(col[2]*255), int(col[3]*255) };
                                        } else if (child->kind == ElementKind::Keybind) {
                                            ImGui::SameLine();
                                            ImGui::Text("[%s]", paris::input::keyboard::vk_name(child->keybind_vk).c_str());
                                        }
                                    }
                                    break;
                                }
                                case ElementKind::SliderInt: {
                                    int v = std::get<int>(el->value);
                                    if (ImGui::SliderInt(el->name.c_str(), &v, el->i_min, el->i_max)) el->value = v;
                                    break;
                                }
                                case ElementKind::SliderDouble: {
                                    float f = float(std::get<double>(el->value));
                                    if (ImGui::SliderFloat(el->name.c_str(), &f, float(el->d_min), float(el->d_max)))
                                        el->value = double(f);
                                    break;
                                }
                                case ElementKind::Input: {
                                    auto& s = std::get<std::string>(el->value);
                                    char buf[512]; std::snprintf(buf, sizeof(buf), "%s", s.c_str());
                                    if (ImGui::InputText(el->name.c_str(), buf, sizeof(buf))) el->value = std::string(buf);
                                    break;
                                }
                                case ElementKind::SingleSelect: {
                                    int v = std::get<int>(el->value);
                                    std::vector<const char*> items;
                                    for (auto& s : el->options) items.push_back(s.c_str());
                                    if (ImGui::Combo(el->name.c_str(), &v, items.data(), int(items.size())))
                                        el->value = v;
                                    break;
                                }
                                case ElementKind::MultiSelect: {
                                    auto& mask = std::get<std::vector<bool>>(el->value);
                                    if (ImGui::TreeNode(el->name.c_str())) {
                                        for (size_t i = 0; i < el->options.size(); ++i) {
                                            bool v = mask[i];
                                            if (ImGui::Checkbox(el->options[i].c_str(), &v)) mask[i] = v;
                                        }
                                        ImGui::TreePop();
                                    }
                                    break;
                                }
                                case ElementKind::Button:
                                    if (ImGui::Button(el->name.c_str()) && el->on_click) el->on_click();
                                    break;
                                case ElementKind::ColorPicker:
                                case ElementKind::Keybind:
                                    // Handled as children of a Checkbox above.
                                    break;
                                case ElementKind::List: {
                                    auto& rows = std::get<std::vector<ListRow>>(el->value);
                                    if (ImGui::BeginListBox(("##" + el->name).c_str())) {
                                        for (auto& r : rows) {
                                            if (r.highlighted) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 80, 255));
                                            ImGui::Selectable((r.name + " — " + r.info).c_str(), false);
                                            if (r.highlighted) ImGui::PopStyleColor();
                                        }
                                        ImGui::EndListBox();
                                    }
                                    break;
                                }
                                }
                            }
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

} // namespace paris::imgui_backend
