#pragma once

#include <cmath>

struct Vec2
{
    Vec2(float a, float b) : x{ a }, y{ b } {}
    Vec2(const Vec2 & a) : x{ a.x }, y{ a.y } {}
    Vec2() {}

    float x, y;
};

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
