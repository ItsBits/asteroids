#include <limits>
#include "AABB.hpp"
#include <cmath>

AABB operator + (const AABB & aabb, const Vec2 & v)
{
    return{
        aabb.getMin() + v,
        aabb.getMax() + v
    };
}

void position_size_from_AABB(const AABB & aabb, Vec2 & position, Vec2 & size)
{
    const auto min = aabb.getMin();
    const auto max = aabb.getMax();

    position.x = (min.x + max.x) / 2.0f;
    position.y = (min.y + max.y) / 2.0f;

    size.x = (max.x - min.x) / 2.0f;
    size.y = (max.y - min.y) / 2.0f;
}

AABB compute_AABB_from_polygon(const std::vector<Vec2> & polygon)
{
    Vec2 mn{  std::numeric_limits<float>::infinity(),  std::numeric_limits<float>::infinity() };
    Vec2 mx{ -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };

    for (const auto & v : polygon)
    {
        mn.x = std::min(mn.x, v.x);
        mn.y = std::min(mn.y, v.y);

        mx.x = std::max(mx.x, v.x);
        mx.y = std::max(mx.y, v.y);
    }

    return { mn, mx };
}

Vec2 AABB_to_size(const AABB & aabb)
{
    const auto mn = aabb.getMin();
    const auto mx = aabb.getMax();

    return {
        std::max(std::abs(mn.x), std::abs(mx.x)),
        std::max(std::abs(mn.y), std::abs(mx.y))
    };
}
