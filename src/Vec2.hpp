#pragma once

#include <vector>

struct Vec2
{
    Vec2(float a, float b) : x{ a }, y{ b } {}
    Vec2(const Vec2 & a) : x{ a.x }, y{ a.y } {}
    Vec2() {}

    float x, y;
};

float dot(Vec2 & v1, Vec2 & v2);

float length(Vec2 & v);

Vec2 operator + (const Vec2 & v, float s);
Vec2 operator - (const Vec2 & v, float s);
Vec2 operator / (const Vec2 & v, float s);
Vec2 operator * (const Vec2 & v, float s);

Vec2 operator + (const Vec2 & v1, const Vec2 & v2);
Vec2 operator - (const Vec2 & v1, const Vec2 & v2);
Vec2 operator / (const Vec2 & v1, const Vec2 & v2);
Vec2 operator * (const Vec2 & v1, const Vec2 & v2);

void wrap_around(Vec2 & point, const Vec2 & size);

Vec2 multiply(const Vec2 & v, const float m[4]);

bool polygons_intersect(const std::vector<Vec2> & a, const std::vector<Vec2> & b);
