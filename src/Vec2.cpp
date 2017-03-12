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

Vec2 multiply(const Vec2 & v, const float m[4])
{
    return {
        v.x * m[0] + v.y * m[1],
        v.x * m[2] + v.y * m[3],
    };
}

bool counter_clock_wise(const Vec2 & a, const Vec2 & b, const Vec2 & c)
{
    // ignoring the collinear case
    return (c.y - a.y) * (b.x - a.x) > (b.y - a.y) * (c.x - a.x);
}

bool segment_intersect(const Vec2 & a1, const Vec2 & a2, const Vec2 & b1, const Vec2 & b2)
{
    // ignoring the collinear case

    const bool c[]
    {
        counter_clock_wise(a1, b1, b2),
        counter_clock_wise(a2, b1, b2),
        counter_clock_wise(a1, a2, b1),
        counter_clock_wise(a1, a2, b2)
    };

    return c[0] != c[1] && c[2] != c[3];
}

bool polygons_intersect(const std::vector<Vec2> & a, const std::vector<Vec2> & b)
{
    // naive algorithm O(n^2) should be the fastest for small n because of tiny overhead compared to other algorithms

    for (std::size_t ia = 0; ia < a.size() - 1; ++ia)
    {
        for (std::size_t ib = 0; ib < b.size() - 1; ++ib)
            if (segment_intersect(a[ia], a[ia + 1], b[ib], b[ib + 1]))
                return true;

        if (segment_intersect(a[ia], a[ia + 1], b[b.size() - 1], b[0]))
            return true;
    }

    for (std::size_t ib = 0; ib < b.size() - 1; ++ib)
        if (segment_intersect(a[a.size() - 1], a[0], b[ib], b[ib + 1]))
            return true;

    if (segment_intersect(a[a.size() - 1], a[0], b[b.size() - 1], b[0]))
        return true;

    return false;
}
