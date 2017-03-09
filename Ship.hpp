#pragma once

#include <GL/gl3w.h>
#include <vector>

#include "Vec2.hpp"
#include "Keyboard.hpp"
#include "Projectile.hpp"

class Ship
{
public:

    Ship(Ship && other)
    {
        m_weapon_cooldown = other.m_weapon_cooldown;

        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;

        m_VAO_bb           = other.m_VAO_bb;
        m_VBO_bb           = other.m_VBO_bb;

        m_position      = other.m_position;
        m_velocity_aka_direction      = other.m_velocity_aka_direction;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_speed = other.m_speed;

        other.m_VAO = 0;
        other.m_VBO = 0;

        other.m_VAO_bb = 0;
        other.m_VBO_bb = 0;
    }

    Ship & operator = (const Ship & other)
    {
        Ship tmp(other);
        *this = std::move(tmp);
        return *this;
    }

    Ship & operator = (Ship && other)
    {

        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteVertexArrays(1, &m_VAO_bb);
        glDeleteBuffers(1, &m_VBO_bb);

        m_weapon_cooldown = other.m_weapon_cooldown;

        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;

        m_VAO_bb           = other.m_VAO_bb;
        m_VBO_bb           = other.m_VBO_bb;

        m_position      = other.m_position;
        m_velocity_aka_direction      = other.m_velocity_aka_direction;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_speed = other.m_speed;

        other.m_VAO = 0;
        other.m_VBO = 0;

        other.m_VAO_bb = 0;
        other.m_VBO_bb = 0;

        return *this;
    }

    bool shoot(std::vector<Projectile> & projectiles, Vec2Gen & rng, float delta_time, float cooldown)
    {
        bool shot = false;

        if (Keyboard::getKeyStatus(GLFW_KEY_SPACE) == Keyboard::KeyStatus::PRESSED && m_weapon_cooldown <= 0.0f)
        {
            m_weapon_cooldown = cooldown;
            projectiles.emplace_back(m_position, m_velocity_aka_direction * 3.0f , 0.02f, 0.5f);
            shot = true;
        }

        if (m_weapon_cooldown > 0.0f) m_weapon_cooldown -= delta_time;

        return shot;
    }

    Ship(float size, float speed) : m_position{ 0.0f, 0.0f }, m_velocity_aka_direction{ 1.0f, 0.0f }
    {
        m_weapon_cooldown = false;

        if (size < 0.0f) size = 0.0f;
        m_size = size;

        if (speed < 0.0f) speed = 0.0f;
        m_speed = speed;

        std::vector<Vec2> mesh{
            {  1.0f,  0.0f },
            { -1.0f, -0.5f },
            { -1.0f,  0.5f },
        };

        m_vertex_count = mesh.size();

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *) (0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(mesh[0]), mesh.data(), GL_STATIC_DRAW);

        upload_bb(); // TODO: render BB somewhere else because all are the same aka. reuse 1 BB
    }

    void draw(GLint offset_uniform, GLint size_uniform, GLint col_uniform, GLint rotation_uniform, bool invincible, bool draw_bb)
    {
        const auto ship_direction_vector = direction();
        const auto ship_direction_angle = -std::atan2(ship_direction_vector.y, ship_direction_vector.x);

        float rotation_matrix[4]{
            std::cos(ship_direction_angle), -std::sin(ship_direction_angle),
            std::sin(ship_direction_angle),  std::cos(ship_direction_angle)
        };

        glUniformMatrix2fv(rotation_uniform, 1, 0, rotation_matrix);

        Vec2 offs[4]{
            { 0.0f, 0.0f },
            { -2.0f, 0.0f },
            { -2.0f, -2.0f },
            { 0.0f, -2.0f }
        };

        // ship
        glUniform1f(size_uniform, m_size);
        if (invincible)
            glUniform3f(col_uniform, 0.0f, 1.0f, 0.5f);
        else
            glUniform3f(col_uniform, 1.0f, 0.0f, 0.7f);

        glBindVertexArray(m_VAO);

        for (const auto & o : offs)
        {
            glUniform2f(offset_uniform, m_position.x + o.x, m_position.y + o.y);
            glDrawArrays(GL_LINE_LOOP, 0, m_vertex_count);
        }
        glBindVertexArray(0);

        if (!draw_bb) return;

        // BB
        glUniform3f(col_uniform, 1.0f, 0.1f, 0.2f);
        glBindVertexArray(m_VAO_bb);
        for (const auto & o : offs)
        {
            glUniform2f(offset_uniform, m_position.x + o.x, m_position.y + o.y);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }
        glBindVertexArray(0);
    }

    Ship(const Ship & other)
    // FUCK STL! this is needed because it is being called by std::vector when reallocating
    {
        m_weapon_cooldown = other.m_weapon_cooldown;

        m_position      = other.m_position;
        m_velocity_aka_direction      = other.m_velocity_aka_direction;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_speed = other.m_speed;

        // ship
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *) (0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBufferData(GL_ARRAY_BUFFER, m_vertex_count * sizeof(Vec2), nullptr, GL_STATIC_DRAW); // RESERVE SPACE

        //glBindBuffer(GL_ARRAY_BUFFER, other.m_VBO); // already hapened before
        glBindBuffer(GL_COPY_READ_BUFFER, other.m_VBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, m_vertex_count * sizeof(Vec2));

        // bb
        glGenVertexArrays(1, &m_VAO_bb);
        glGenBuffers(1, &m_VBO_bb);

        glBindVertexArray(m_VAO_bb);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO_bb);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *) (0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vec2), nullptr, GL_STATIC_DRAW); // RESERVE SPACE

        //glBindBuffer(GL_ARRAY_BUFFER, other.m_VBO); // already hapened before
        glBindBuffer(GL_COPY_READ_BUFFER, other.m_VBO_bb);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, 4 * sizeof(Vec2));

    }

    void move(float delta_time, float rotation_speed_RPS, float movement_speed)
    {
        m_speed = movement_speed;

        // rotation
        if (Keyboard::getKeyStatus(GLFW_KEY_RIGHT) != Keyboard::getKeyStatus(GLFW_KEY_LEFT))
        {

            if (Keyboard::getKeyStatus(GLFW_KEY_RIGHT) == Keyboard::KeyStatus::PRESSED)
            {
                const auto c = std::cos(2.0f * 3.1415f * -rotation_speed_RPS * delta_time);
                const auto s = std::sin(2.0f * 3.1415f * -rotation_speed_RPS * delta_time);

                Vec2 new_v;

                new_v.x = m_velocity_aka_direction.x * c - m_velocity_aka_direction.y * s;
                new_v.y = m_velocity_aka_direction.x * s + m_velocity_aka_direction.y * c;

                m_velocity_aka_direction.x = new_v.x;
                m_velocity_aka_direction.y = new_v.y;
            }
            else
            {
                const auto c = std::cos(2.0f * 3.1415f * rotation_speed_RPS * delta_time);
                const auto s = std::sin(2.0f * 3.1415f * rotation_speed_RPS * delta_time);

                Vec2 new_v;

                new_v.x = m_velocity_aka_direction.x * c - m_velocity_aka_direction.y * s;
                new_v.y = m_velocity_aka_direction.x * s + m_velocity_aka_direction.y * c;

                m_velocity_aka_direction.x = new_v.x;
                m_velocity_aka_direction.y = new_v.y;
            }

            // normalize just in case
            m_velocity_aka_direction = m_velocity_aka_direction / length(m_velocity_aka_direction);
        }

        // back and forth

        // euler integration
        if (Keyboard::getKeyStatus(GLFW_KEY_UP) != Keyboard::getKeyStatus(GLFW_KEY_DOWN))
            if (Keyboard::getKeyStatus(GLFW_KEY_UP) == Keyboard::KeyStatus::PRESSED)
            {
                m_position.x += m_velocity_aka_direction.x * m_speed * delta_time;
                m_position.y += m_velocity_aka_direction.y * m_speed * delta_time;
            }
            else
            {
                m_position.x -= m_velocity_aka_direction.x * m_speed * delta_time;
                m_position.y -= m_velocity_aka_direction.y * m_speed * delta_time;
            }

        // wrap around
        const auto box = boundingBox();
        const auto min = box.getMin();

        if (min.x < -1.0f) m_position.x += 2.0f;
        if (min.y < -1.0f) m_position.y += 2.0f;

        if (min.x > 1.0f) m_position.x -= 2.0f;
        if (min.y > 1.0f) m_position.y -= 2.0f;
    }

    Vec2 direction() const
    {
        return m_velocity_aka_direction;
    }

    AABB boundingBox()
    {
        return AABB{
            Vec2{ -m_size + m_position.x, -m_size  + m_position.y },
            Vec2{  m_size + m_position.x,  m_size  + m_position.y }
        };
    }

    ~Ship()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteVertexArrays(1, &m_VAO_bb);
        glDeleteBuffers(1, &m_VBO_bb);
    }

private:
    GLuint m_VAO, m_VBO;
    GLuint m_VAO_bb, m_VBO_bb;
    GLsizei m_vertex_count;
    float m_size;

    float m_weapon_cooldown;

    Vec2 m_position;
    Vec2 m_velocity_aka_direction;
    float m_speed; // velocity * speed = actually speed

    void upload_bb()
    {
        std::vector<Vec2> bb{
            Vec2{ -1.0f, -1.0f },
            Vec2{  1.0f, -1.0f },
            Vec2{  1.0f,  1.0f },
            Vec2{ -1.0f,  1.0f }
        };


        glGenVertexArrays(1, &m_VAO_bb);
        glGenBuffers(1, &m_VBO_bb);

        glBindVertexArray(m_VAO_bb);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO_bb);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *) (0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBufferData(GL_ARRAY_BUFFER, bb.size() * sizeof(bb[0]), bb.data(), GL_STATIC_DRAW);
    }

};