#include "AABB.hpp"

AABB operator + (const AABB & aabb, const Vec2 & v)
{
    return{
        aabb.getMin() + v,
        aabb.getMax() + v
    };
}