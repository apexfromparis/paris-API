#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <string>

namespace paris {

// Two names for the same thing: `vec2_t` matches the Paris docs, `vector2` matches
// the perception.cx docs. Both are aliases into the same struct.

struct vector2 {
    float x = 0.f, y = 0.f;

    vector2() = default;
    vector2(float x_, float y_) : x(x_), y(y_) {}

    vector2 operator+(const vector2& o) const { return { x + o.x, y + o.y }; }
    vector2 operator-(const vector2& o) const { return { x - o.x, y - o.y }; }
    vector2 operator*(float s)          const { return { x * s,   y * s   }; }
    vector2 operator/(float s)          const { return { x / s,   y / s   }; }
    bool    operator==(const vector2& o) const { return x == o.x && y == o.y; }

    float   length()      const { return std::sqrt(x*x + y*y); }
    float   length_sqr()  const { return x*x + y*y; }
    float   distance(const vector2& o) const { return (*this - o).length(); }
    float   dot(const vector2& o)      const { return x*o.x + y*o.y; }
    vector2 normalized() const {
        float l = length();
        return l > 1e-6f ? vector2{ x / l, y / l } : vector2{};
    }
    std::string to_string() const {
        return "vector2(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};
using vec2_t = vector2;

struct vector3 {
    float x = 0.f, y = 0.f, z = 0.f;

    vector3() = default;
    vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    vector3 operator+(const vector3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    vector3 operator-(const vector3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    vector3 operator*(float s)          const { return { x * s,   y * s,   z * s   }; }
    vector3 operator/(float s)          const { return { x / s,   y / s,   z / s   }; }
    bool    operator==(const vector3& o) const { return x == o.x && y == o.y && z == o.z; }

    float   length()      const { return std::sqrt(x*x + y*y + z*z); }
    float   length_sqr()  const { return x*x + y*y + z*z; }
    float   distance(const vector3& o) const { return (*this - o).length(); }
    float   dot(const vector3& o)      const { return x*o.x + y*o.y + z*o.z; }
    vector3 cross(const vector3& o)    const {
        return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
    }
    vector3 normalized() const {
        float l = length();
        return l > 1e-6f ? vector3{ x / l, y / l, z / l } : vector3{};
    }

    // Backend-dependent projection. Defined in render.cpp.
    bool to_screen(vector2& out) const;

    std::string to_string() const {
        return "vector3(" + std::to_string(x) + ", " + std::to_string(y)
                          + ", " + std::to_string(z) + ")";
    }
};
using vec3_t = vector3;

// RGBA 0-255 per channel — matches the Paris `color_t` shape.
struct color_t {
    int r = 255, g = 255, b = 255, a = 255;

    color_t() = default;
    color_t(int r_, int g_, int b_, int a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}

    color_t with_alpha(int new_a) const { return { r, g, b, new_a }; }

    color_t lerp(const color_t& o, float t) const {
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        auto mix = [t](int a, int b){ return int(a + (b - a) * t); };
        return { mix(r, o.r), mix(g, o.g), mix(b, o.b), mix(a, o.a) };
    }

    static color_t from_hsv(float h, float s, float v) {
        if (s <= 0.f) { int c = int(v * 255.f); return { c, c, c, 255 }; }
        h = std::fmod(h, 1.f) * 6.f;
        int  i = int(h);
        float f = h - i;
        float p = v * (1.f - s);
        float q = v * (1.f - s * f);
        float t = v * (1.f - s * (1.f - f));
        float r_, g_, b_;
        switch (i) {
            case 0: r_=v;  g_=t;  b_=p;  break;
            case 1: r_=q;  g_=v;  b_=p;  break;
            case 2: r_=p;  g_=v;  b_=t;  break;
            case 3: r_=p;  g_=q;  b_=v;  break;
            case 4: r_=t;  g_=p;  b_=v;  break;
            default:r_=v;  g_=p;  b_=q;  break;
        }
        return { int(r_*255.f), int(g_*255.f), int(b_*255.f), 255 };
    }

    uint32_t rgba() const {
        return (uint32_t(a) << 24) | (uint32_t(b) << 16)
             | (uint32_t(g) << 8)  |  uint32_t(r);
    }
};

// Rotation as unit quaternion: w + xi + yj + zk. Stored xyz first, w last so the
// memory layout matches most game engines (Source, Unreal, Unity).
struct quaternion {
    float x = 0.f, y = 0.f, z = 0.f, w = 1.f;

    quaternion() = default;
    quaternion(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}

    static quaternion identity() { return { 0, 0, 0, 1 }; }

    // Euler in radians, XYZ order (yaw around Z when using engine convention below).
    static quaternion from_euler(float rx, float ry, float rz) {
        float cx = std::cos(rx * 0.5f), sx = std::sin(rx * 0.5f);
        float cy = std::cos(ry * 0.5f), sy = std::sin(ry * 0.5f);
        float cz = std::cos(rz * 0.5f), sz = std::sin(rz * 0.5f);
        return {
            sx*cy*cz - cx*sy*sz,
            cx*sy*cz + sx*cy*sz,
            cx*cy*sz - sx*sy*cz,
            cx*cy*cz + sx*sy*sz
        };
    }

    vector3 to_euler() const {
        // XYZ intrinsic, matches from_euler.
        float sinp = 2.f * (w*y - z*x);
        float pitch = std::fabs(sinp) >= 1.f
            ? std::copysign(1.5707963f, sinp)
            : std::asin(sinp);
        float roll  = std::atan2(2.f * (w*x + y*z), 1.f - 2.f * (x*x + y*y));
        float yaw   = std::atan2(2.f * (w*z + x*y), 1.f - 2.f * (y*y + z*z));
        return { roll, pitch, yaw };
    }

    quaternion operator*(const quaternion& o) const {
        return {
            w*o.x + x*o.w + y*o.z - z*o.y,
            w*o.y - x*o.z + y*o.w + z*o.x,
            w*o.z + x*o.y - y*o.x + z*o.w,
            w*o.w - x*o.x - y*o.y - z*o.z
        };
    }

    vector3 rotate(const vector3& v) const {
        vector3 u{ x, y, z };
        vector3 t = u.cross(v) * 2.f;
        return v + (t * w) + u.cross(t);
    }

    quaternion inverse() const {
        float n = x*x + y*y + z*z + w*w;
        if (n <= 1e-8f) return { 0, 0, 0, 1 };
        return { -x/n, -y/n, -z/n, w/n };
    }
};

// Row-major 4x4. m[row][col]. That matches DirectX / most game math and lets us
// stream directly into HLSL constant buffers without a transpose.
struct matrix4x4 {
    std::array<std::array<float, 4>, 4> m{};

    matrix4x4() {
        for (int i = 0; i < 4; ++i) m[i][i] = 1.f;
    }

    float& at(int r, int c)             { return m[r][c]; }
    float  at(int r, int c) const       { return m[r][c]; }

    static matrix4x4 identity() { return {}; }

    static matrix4x4 translation(const vector3& t) {
        matrix4x4 r;
        r.m[0][3] = t.x; r.m[1][3] = t.y; r.m[2][3] = t.z;
        return r;
    }

    static matrix4x4 scaling(const vector3& s) {
        matrix4x4 r;
        r.m[0][0] = s.x; r.m[1][1] = s.y; r.m[2][2] = s.z;
        return r;
    }

    static matrix4x4 rotation(const quaternion& q) {
        matrix4x4 r;
        float xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z;
        float xy = q.x*q.y, xz = q.x*q.z, yz = q.y*q.z;
        float wx = q.w*q.x, wy = q.w*q.y, wz = q.w*q.z;
        r.m[0][0] = 1 - 2*(yy+zz); r.m[0][1] =     2*(xy-wz); r.m[0][2] =     2*(xz+wy);
        r.m[1][0] =     2*(xy+wz); r.m[1][1] = 1 - 2*(xx+zz); r.m[1][2] =     2*(yz-wx);
        r.m[2][0] =     2*(xz-wy); r.m[2][1] =     2*(yz+wx); r.m[2][2] = 1 - 2*(xx+yy);
        return r;
    }

    matrix4x4 operator*(const matrix4x4& o) const {
        matrix4x4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                float s = 0.f;
                for (int k = 0; k < 4; ++k) s += m[i][k] * o.m[k][j];
                r.m[i][j] = s;
            }
        return r;
    }

    vector3 transform(const vector3& v) const {
        return {
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3],
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3],
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]
        };
    }
};

} // namespace paris
