#pragma once

#include <GL/gl3w.h>
#include <vector>
#include <cassert>

#include "Vec2.hpp"
#include "AABB.hpp"
#include "Polygon.hpp"

class Projectile
{
public:
    Projectile(Vec2 position, Vec2 velocity, float size, float time_til_death) :
        m_position{ position },
        m_velocity{ velocity }
    {
        if (size < 0.0f) size = 0.0f;
        m_size = size;

        if (time_til_death < 0.0f) time_til_death = 0.0f;
        m_time_til_death = time_til_death;

        std::vector<Vec2> mesh{
            {  -1.0f,  -1.0f },
            {  -1.0f,   1.0f },
            {   1.0f,   1.0f },
            {   1.0f,  -1.0f }
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
    }

    Projectile(Projectile && other)
    {
        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;
        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_time_til_death = other.m_time_til_death;

        other.m_VAO = 0;
        other.m_VBO = 0;
    }

    Projectile & operator = (const Projectile & other)
    {
        Projectile tmp(other);
        *this = std::move(tmp);
        return *this;
    }

    Projectile & operator = (Projectile && other)
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);

        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;
        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_time_til_death = other.m_time_til_death;

        other.m_VAO = 0;
        other.m_VBO = 0;

        return *this;
    }

    Projectile(const Projectile & other)
    // FUCK STL! this is needed because it is being called by std::vector when reallocating
    {
        //m_VAO           = other.m_VAO;
        //m_VBO           = other.m_VBO;
        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;
        m_time_til_death = other.m_time_til_death;

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
    }

    AABB boundingBox()
    {
        return AABB{
            Vec2{ -m_size + m_position.x, -m_size  + m_position.y },
            Vec2{  m_size + m_position.x,  m_size  + m_position.y }
        };
    }

    bool dead() const
    {
        return m_time_til_death < 0.0f;
    }

    void markDead()
    {
        m_time_til_death = -1.0f;
    }

    void move(float delta_time)
    {
        m_time_til_death -= delta_time;

        // euler
        m_position.x += m_velocity.x * delta_time;
        m_position.y += m_velocity.y * delta_time;

        const auto box = boundingBox();
        const auto min = box.getMin();

        if (min.x < -1.0f) m_position.x += 2.0f;
        if (min.y < -1.0f) m_position.y += 2.0f;

        if (min.x > 1.0f) m_position.x -= 2.0f;
        if (min.y > 1.0f) m_position.y -= 2.0f;
    }

    ~Projectile() // TODO: fix the fact that shuffling vectors will call destructors
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }

    void draw(GLint offset_uniform, GLint size_uniform, GLint col_uniform)
    {
        assert(m_time_til_death >= 0.0f);

        Vec2 offs[4]{
            { 0.0f, 0.0f },
            { -2.0f, 0.0f },
            { -2.0f, -2.0f },
            { 0.0f, -2.0f }
        };

        // asteroid
        glUniform2f(size_uniform, m_size, m_size);
        glUniform3f(col_uniform, 0.0f, 0.0f, 1.0f);
        glBindVertexArray(m_VAO);
        { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }
        for (const auto & o : offs)
        {
            glUniform2f(offset_uniform, m_position.x + o.x, m_position.y + o.y);
            glDrawArrays(GL_LINE_LOOP, 0, m_vertex_count);
            { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }
        }
        { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }

        glBindVertexArray(0);
        { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }
    }

private:
    GLuint  m_VAO, m_VBO; // TODO: move mesh out of this class, because it's the same all the time
    Vec2 m_position;
    Vec2 m_velocity;
    float m_size;
    [[deprecated]]
    GLsizei m_vertex_count;
    float m_time_til_death;

};