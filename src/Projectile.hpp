#pragma once

#include <GL/gl3w.h>

#include <vector>
#include <cassert>

#include "Vec2.hpp"
#include "AABB.hpp"
#include "Polygon.hpp"

static const std::vector<Vec2> DEFAULT_PROJECTILE_MODEL
{
    { -1.0f, -1.0f },
    {  1.0f, -1.0f },
    {  1.0f,  1.0f },
    { -1.0f,  1.0f },
};

class Projectile
{
public:
    //==========================================================================
    Projectile() : m_time_left{ 0.0f } {}

    //==========================================================================
    Projectile(Vec2 position, Vec2 velocity, Vec2 size, float life_time) :
        m_size     { std::max(0.0f, size.x), std::max(0.0f, size.y) },
        m_position { position },
        m_velocity { velocity },
        m_time_left{ std::max(0.0f, life_time) },
        m_polygon  { DEFAULT_PROJECTILE_MODEL }
    {
        const auto angle = -std::atan2(m_velocity.y, m_velocity.x);

        m_rotation_matrix[0] =  std::cos(angle);
        m_rotation_matrix[1] = -std::sin(angle);
        m_rotation_matrix[2] =  std::sin(angle);
        m_rotation_matrix[3] =  std::cos(angle);

        // calculate AABB
        std::vector<Vec2> polygon = m_polygon.vertices();
        // scale and rotate
        std::for_each(polygon.begin(), polygon.end(), [this](Vec2 & v){ v = multiply(v * m_size, m_rotation_matrix); });

        const Vec2 size2 = AABB_to_size(compute_AABB_from_polygon(polygon));
        m_bounding_box = AABB{ { -size2.x, -size2.y }, size2 }; // symmetric AABB
    }

    //==========================================================================
    Projectile & operator = (const Projectile & other)
    {
        Projectile tmp{ other };
        *this = std::move(tmp);
        return *this;
    }

    //==========================================================================
    Projectile(Projectile && other) :
        m_size     { other.m_size },
        m_position { other.m_position },
        m_velocity { other.m_velocity },
        m_time_left{ other.m_time_left },
        m_polygon  { std::move(other.m_polygon) },
        m_bounding_box { other.m_bounding_box }
    {
        std::copy(std::begin(other.m_rotation_matrix), std::end(other.m_rotation_matrix), std::begin(m_rotation_matrix));

        other.m_size      = Vec2{ 0.0f, 0.0f };
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_velocity  = Vec2{ 0.0f, 0.0f };
        other.m_time_left = 0.0f;
        other.m_bounding_box  = AABB{ Vec2{ 0.0f, 0.0f }, Vec2{ 0.0f, 0.0f } };
        for (auto & i : other.m_rotation_matrix) i = 0.0f;
    }

    //==========================================================================
    Projectile & operator = (Projectile && other)
    {
        m_size      = other.m_size;
        m_position  = other.m_position;
        m_velocity  = other.m_velocity;
        m_time_left = other.m_time_left;
        m_polygon   = std::move(other.m_polygon);
        m_bounding_box = other.m_bounding_box;
        std::copy(std::begin(other.m_rotation_matrix), std::end(other.m_rotation_matrix), std::begin(m_rotation_matrix));

        other.m_size      = Vec2{ 0.0f, 0.0f };
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_velocity  = Vec2{ 0.0f, 0.0f };
        other.m_time_left = 0.0f;
        other.m_bounding_box  = AABB{ Vec2{ 0.0f, 0.0f }, Vec2{ 0.0f, 0.0f } };
        for (auto & i : other.m_rotation_matrix) i = 0.0f;

        return *this;
    }

    //==========================================================================
    Projectile(const Projectile & other) = default;

    //==========================================================================
    ~Projectile() = default;

    //==========================================================================
    bool isDead() const
    {
        return m_time_left <= 0.0f;
    }

    //==========================================================================
    void kill()
    {
        m_time_left = -1.0f;
    }

    //==========================================================================
    void draw(GLint scale_uniform, GLint rotation_uniform, GLint translation_uniform)
    {
        assert(m_time_left > 0.0f);

        glUniform2f(scale_uniform, m_size.x, m_size.y);
        glUniformMatrix2fv(rotation_uniform, 1, GL_FALSE, m_rotation_matrix);
        glUniform2f(translation_uniform, m_position.x, m_position.y);

        m_polygon.draw();
    }

    //==========================================================================
    void move(float delta_time)
    {
        // euler integration
        m_position.x += m_velocity.x * delta_time;
        m_position.y += m_velocity.y * delta_time;

        // wrap/warp around
        wrap_around(m_position, m_size);

        m_time_left -= delta_time;
    }

    //==========================================================================
    AABB boundingBox() const
    {
        return {
            m_bounding_box.getMin() + m_position,
            m_bounding_box.getMax() + m_position
        };
    }

    //==========================================================================
    const std::vector<Vec2> & polygon() const
    {
        return m_polygon.vertices();
    }

    //==========================================================================
    std::vector<Vec2> polygonSRT() const
    {
        const auto & vertices = m_polygon.vertices();

        std::vector<Vec2> result;
        result.reserve(vertices.size());

        for (const auto & v : vertices)
            result.emplace_back(multiply(v, m_rotation_matrix) * m_size + m_position);

        return result;
    }

private:
    Vec2 m_size; // TODO: generalize Object (AABB size position velocity rotation matrix...)
    Vec2 m_position;
    Vec2 m_velocity;

    float m_time_left;

    Polygon m_polygon;

    AABB m_bounding_box;
    float m_rotation_matrix[4];

};
