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
    Projectile(Vec2 position, Vec2 velocity, Vec2 size, float life_time) :
        m_size     { std::max(0.0f, size.x), std::max(0.0f, size.y) },
        m_position { position },
        m_velocity { velocity },
        m_time_left{ std::max(0.0f, life_time) },
        m_polygon  { DEFAULT_PROJECTILE_MODEL }
    {}

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
        m_polygon  { std::move(other.m_polygon) }
    {
        other.m_size      = Vec2{ 0.0f, 0.0f };
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_velocity  = Vec2{ 0.0f, 0.0f };
        other.m_time_left = 0.0f;
    }

    //==========================================================================
    Projectile & operator = (Projectile && other)
    {
        m_size      = other.m_size;
        m_position  = other.m_position;
        m_velocity  = other.m_velocity;
        m_time_left = other.m_time_left;
        m_polygon   = std::move(other.m_polygon);

        other.m_size      = Vec2{ 0.0f, 0.0f };
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_velocity  = Vec2{ 0.0f, 0.0f };
        other.m_time_left = 0.0f;

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

        const auto angle = -std::atan2(m_velocity.y, m_velocity.x);

        const float rotation_matrix[]{ // TODO: pre-compute, because it stays constant
            std::cos(angle), -std::sin(angle),
            std::sin(angle),  std::cos(angle)
        };

        glUniform2f(scale_uniform, m_size.x, m_size.y);
        glUniformMatrix2fv(rotation_uniform, 1, GL_FALSE, rotation_matrix);
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


    /*
     *
     *
     *
     *
     * refactor stuff below
     *
     *
     *
     */



    AABB boundingBox() const // TODO: this is not always correct because of rotation
    {
        return {
            Vec2{ m_position.x - m_size.x, m_position.y - m_size.y },
            Vec2{ m_position.x + m_size.x, m_position.y + m_size.y }
        };
    }




private:
    Vec2 m_size;
    Vec2 m_position;
    Vec2 m_velocity;

    float m_time_left;

    Polygon m_polygon;

};