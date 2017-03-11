#pragma once

#include <GL/gl3w.h>
#include <vector>

#include "Vec2.hpp"
#include "Keyboard.hpp"
#include "Projectile.hpp"

static const std::vector<Vec2> DEFAULT_SHIP_MODEL
{
    {  1.0f,  0.0f },
    { -1.0f, -0.5f },
    { -1.0f,  0.5f },
};

class Ship
{
public:
    //==========================================================================
    Ship(float size, float movement_speed, float rotation_speed) :
        m_size     { std::max(0.0f, size) },
        m_position { 0.0f, 0.0f },
        m_direction{ 1.0f, 0.0f },
        m_cool_down{ 0.0f },
        m_movement_speed{ std::max(0.0f, movement_speed) },
        m_rotation_speed{ std::max(0.0f, rotation_speed) },
        m_polygon  { DEFAULT_SHIP_MODEL }
    {}

    //==========================================================================
    Ship & operator = (const Ship & other)
    {
        Ship tmp{ other };
        *this = std::move(tmp);
        return *this;
    }

    //==========================================================================
    Ship(Ship && other) :
        m_size     { other.m_size      },
        m_position { other.m_position  },
        m_direction{ other.m_direction },
        m_cool_down{ other.m_cool_down },
        m_movement_speed{ other.m_movement_speed },
        m_rotation_speed{ other.m_rotation_speed },
        m_polygon  { std::move(other.m_polygon)  }
    {
        other.m_size      = 1.0f;
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_direction = Vec2{ 0.0f, 0.0f };
        other.m_cool_down = 0.0f;
        other.m_movement_speed = 0.0f;
        other.m_rotation_speed = 0.0f;
    }

    //==========================================================================
    Ship & operator = (Ship && other)
    {
        m_size      = other.m_size;
        m_position  = other.m_position;
        m_direction = other.m_direction;
        m_cool_down = other.m_cool_down;
        m_movement_speed = other.m_movement_speed;
        m_rotation_speed = other.m_rotation_speed;
        m_polygon   = std::move(other.m_polygon);

        other.m_size      = 1.0f;
        other.m_position  = Vec2{ 0.0f, 0.0f };
        other.m_direction = Vec2{ 0.0f, 0.0f };
        other.m_cool_down = 0.0f;
        other.m_movement_speed = 0.0f;
        other.m_rotation_speed = 0.0f;

        return *this;
    }

    //==========================================================================
    Ship(const Ship & other) = default;

    //==========================================================================
    ~Ship() = default;

    //==========================================================================
    void move(float delta_time)
    {
        // using euler integration

        // rotation
        if (Keyboard::getKeyStatus(GLFW_KEY_RIGHT) != Keyboard::getKeyStatus(GLFW_KEY_LEFT))
        {
            // determine rotation direction
            const auto right = Keyboard::getKeyStatus(GLFW_KEY_RIGHT) == Keyboard::KeyStatus::PRESSED;

            // rotation matrix elements
            const auto cs = std::cos(6.2831853f * m_rotation_speed * delta_time);
            const auto sn = std::sin(6.2831853f * m_rotation_speed * delta_time) * (right ? -1.0f : 1.0f);

            const auto old_direction = m_direction;

            // rotate direction (rotation-matrix vector multiplication)
            m_direction.x = old_direction.x * cs - old_direction.y * sn;
            m_direction.y = old_direction.x * sn + old_direction.y * cs;

            // normalize direction
            m_direction = m_direction / length(m_direction);
        }

        // back and forwards
        if (Keyboard::getKeyStatus(GLFW_KEY_UP) != Keyboard::getKeyStatus(GLFW_KEY_DOWN))
            if (Keyboard::getKeyStatus(GLFW_KEY_UP) == Keyboard::KeyStatus::PRESSED)
            {
                m_position.x += m_direction.x * m_movement_speed * delta_time;
                m_position.y += m_direction.y * m_movement_speed * delta_time;
            }
            else
            {
                m_position.x -= m_direction.x * m_movement_speed * delta_time;
                m_position.y -= m_direction.y * m_movement_speed * delta_time;
            }

        // TODO: update bounding box because of rotation (m_size is not sufficient)

        // wrap/warp around
        wrap_around(m_position, { m_size, m_size });
    }

    //==========================================================================
    void draw(GLint scale_uniform, GLint rotation_uniform, GLint translation_uniform)
    {
        const auto angle = -std::atan2(m_direction.y, m_direction.x);

        const float rotation_matrix[]{
            std::cos(angle), -std::sin(angle),
            std::sin(angle),  std::cos(angle)
        };

        glUniform2f(scale_uniform, m_size, m_size);
        glUniformMatrix2fv(rotation_uniform, 1, GL_FALSE, rotation_matrix);
        glUniform2f(translation_uniform, m_position.x, m_position.y);

        m_polygon.draw();
    }


    /*
     *
     *
     *
     * refactor stuff below
     */




    bool shoot(std::vector<Projectile> & projectiles, Vec2Gen & rng, float delta_time, float cooldown)
    {
        bool shot = false;

        if (Keyboard::getKeyStatus(GLFW_KEY_SPACE) == Keyboard::KeyStatus::PRESSED && m_cool_down <= 0.0f)
        {
            m_cool_down = cooldown;
            projectiles.emplace_back(m_position, m_direction * 0.6f , Vec2{ 0.03f, 0.01f }, 1.5f);
            shot = true;
        }

        if (m_cool_down > 0.0f) m_cool_down -= delta_time;

        return shot;
    }

    AABB boundingBox() const // TODO: this is not always correct because of rotation
    {
        return AABB{
            Vec2{ m_position.x - m_size, m_position.y - m_size },
            Vec2{ m_position.x + m_size, m_position.y + m_size }
        };
    }

private:
    float m_size;
    Vec2 m_position;
    Vec2 m_direction;

    float m_cool_down;
    float m_movement_speed;
    float m_rotation_speed;

    Polygon m_polygon;

};