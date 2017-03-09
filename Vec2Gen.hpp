#pragma once

#include <random>

#include "Vec2.hpp"

class Vec2Gen
{
public:
    Vec2Gen(
        std::minstd_rand::result_type seed = std::minstd_rand::default_seed
    ) :
        generator{ seed },
        // according to standard this is a half range, but implementations use inclusive range
        x_distribution{ -1.0f, 1.0f },
        y_distribution{ -1.0f, 1.0f }
    {}

    Vec2 get()
    {
        return Vec2{ x_distribution(generator), y_distribution(generator) };
    }

private:
    std::minstd_rand generator;

    std::uniform_real_distribution<float> x_distribution;
    std::uniform_real_distribution<float> y_distribution;

};
