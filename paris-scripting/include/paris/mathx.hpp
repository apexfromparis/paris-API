#pragma once

#include "types.hpp"

// Extended math on top of types.hpp — constants, scalar helpers, and small
// utilities that show up over and over in game code.

namespace paris::mathx {

constexpr double M_PI    = 3.14159265358979323846;
constexpr double M_TAU   = 6.28318530717958647692;
constexpr double DEG2RAD = M_PI / 180.0;
constexpr double RAD2DEG = 180.0 / M_PI;

template <typename T> constexpr T clamp   (T v, T mn, T mx)                { return v < mn ? mn : (v > mx ? mx : v); }
template <typename T> constexpr T saturate(T v)                            { return clamp(v, T(0), T(1)); }
template <typename T> constexpr T lerp    (T a, T b, float t)              { return a + (b - a) * t; }
template <typename T> constexpr T inverse_lerp(T a, T b, T v)              { return b == a ? T(0) : (v - a) / (b - a); }

inline float smoothstep(float e0, float e1, float x) {
    float t = clamp((x - e0) / (e1 - e0), 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

inline float remap(float x, float in_min, float in_max, float out_min, float out_max) {
    float t = inverse_lerp(in_min, in_max, x);
    return lerp(out_min, out_max, t);
}

inline float wrap(float x, float min, float max) {
    float range = max - min;
    if (range <= 0.f) return min;
    float r = std::fmod(x - min, range);
    if (r < 0.f) r += range;
    return r + min;
}

// mat4 factory namespace — matches perception.cx's `mat4.*` convention.
namespace mat4 {
    inline matrix4x4 identity()                     { return matrix4x4::identity(); }
    inline matrix4x4 translation(const vector3& t)  { return matrix4x4::translation(t); }
    inline matrix4x4 scaling(const vector3& s)      { return matrix4x4::scaling(s); }
    inline matrix4x4 rotation(const quaternion& q)  { return matrix4x4::rotation(q); }
}

} // namespace paris::mathx
