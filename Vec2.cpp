#include "Vec2.hpp"

#include <cmath>

float dot(Vec2 & v1, Vec2 & v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

float length(Vec2 & v)
{
    return std::sqrt(dot(v, v));
}

Vec2 operator + (const Vec2 & v, float s) { return Vec2{ v.x + s, v.y + s}; }
Vec2 operator - (const Vec2 & v, float s) { return Vec2{ v.x - s, v.y - s}; }
Vec2 operator / (const Vec2 & v, float s) { return Vec2{ v.x / s, v.y / s}; }
Vec2 operator * (const Vec2 & v, float s) { return Vec2{ v.x * s, v.y * s}; }

Vec2 operator + (const Vec2 & v1, const Vec2 & v2) { return Vec2{ v1.x + v2.x, v1.y + v2.y}; }
Vec2 operator - (const Vec2 & v1, const Vec2 & v2) { return Vec2{ v1.x - v2.x, v1.y - v2.y}; }
Vec2 operator / (const Vec2 & v1, const Vec2 & v2) { return Vec2{ v1.x / v2.x, v1.y / v2.y}; }
Vec2 operator * (const Vec2 & v1, const Vec2 & v2) { return Vec2{ v1.x * v2.x, v1.y * v2.y}; }

void wrap_around(Vec2 & point, const Vec2 & size)
{
    if (point.x < -1.0f - size.x) point.x += 2.0f + 2.0f * size.x;
    if (point.y < -1.0f - size.y) point.y += 2.0f + 2.0f * size.y;
    if (point.x >  1.0f + size.x) point.x -= 2.0f + 2.0f * size.x;
    if (point.y >  1.0f + size.y) point.y -= 2.0f + 2.0f * size.y;
}

Vec2 multiply(const Vec2 & v, float m[4])
{
    return {
        v.x * m[0] + v.y * m[1],
        v.x * m[2] + v.y * m[3],
    };
}
