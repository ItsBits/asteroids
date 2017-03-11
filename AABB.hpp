#pragma once

#include <iostream>
#include <vector>
#include "Vec2.hpp"

struct AABB
{
public:
    AABB(const Vec2 & mi, const Vec2 & ma) : min{ mi }, max{ ma } {}
    AABB(const AABB & a) : min{ a.min }, max{ a.max } {}
    AABB() {}

    static bool intersect(const AABB & a, const AABB & b)
    {
        bool result = true;

        result = result && a.min.x <= b.max.x;
        result = result && a.max.x >= b.min.x;
        result = result && a.min.y <= b.max.y;
        result = result && a.max.y >= b.min.y;

        return result;
    }

    void print()
    {
        std::cout << min.x << " | " << min.y << std::endl;
        std::cout << max.x << " | " << max.y << std::endl;
    }

    Vec2 getMin() const { return min; }
    Vec2 getMax() const { return max; }

private:
    Vec2 min;
    Vec2 max;

};

AABB operator + (const AABB & aabb, const Vec2 & v);

void position_size_from_AABB(const AABB & aabb, Vec2 & position, Vec2 & size);

AABB compute_ABB_from_polygon(const std::vector<Vec2> & polygon);

Vec2 AABB_to_size(const AABB & aabb);