#include "AABB.hpp"

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
