#pragma once

#include <cstddef>
#include <vector>
#include <cmath>

#include <random>
#include <algorithm>

#include "Vec2.hpp"
#include "Vec2Gen.hpp"
#include "AABB.hpp"

#include <GL/gl3w.h>

class Rock
{
public:

    float size() const { return m_size; }

    Rock(Rock && other)
    {
        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;

        m_VAO_bb           = other.m_VAO_bb;
        m_VBO_bb           = other.m_VBO_bb;

        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;

        other.m_VAO = 0;
        other.m_VBO = 0;

        other.m_VAO_bb = 0;
        other.m_VBO_bb = 0;
    }

    Rock & operator = (const Rock & other)
    {
        Rock tmp(other);
        *this = std::move(tmp);
        return *this;
    }

    Rock & operator = (Rock && other)
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteVertexArrays(1, &m_VAO_bb);
        glDeleteBuffers(1, &m_VBO_bb);

        m_VAO           = other.m_VAO;
        m_VBO           = other.m_VBO;

        m_VAO_bb           = other.m_VAO_bb;
        m_VBO_bb           = other.m_VBO_bb;

        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;

        other.m_VAO = 0;
        other.m_VBO = 0;

        other.m_VAO_bb = 0;
        other.m_VBO_bb = 0;

        return *this;
    }

    Rock split(Vec2Gen & rng)
    {
        return Rock{
            m_size / 2.0f, m_vertex_count / 2, rng, m_position, rng.get(), 0.5f
        };
    }

    Rock(float size, int vertex_count, Vec2Gen & rng, Vec2 position, Vec2 velocity, float roughness) :
        m_position{ position },
        m_velocity{ velocity }
    {
        if (vertex_count < 3) vertex_count = 3;
        if (size < 0.0f) size = 0.0f;

        roughness = std::min(1.0f, std::max(0.0f, roughness));

        m_vertex_count = vertex_count;
        m_size = size;

        std::vector<Vec2> vertices;

        vertices.reserve(static_cast<std::size_t>(vertex_count));

        // TODO: fix the fact that this does not guarantee a planar graph

        // generate points
        while (vertex_count--)
            vertices.push_back(rng.get());

        auto smooth_out = [roughness](Vec2 & v) // well, that was not the best idea ever
        {
            v.x = std::copysign(std::pow(std::abs(v.x), roughness), v.x);
            v.y = std::copysign(std::pow(std::abs(v.y), roughness), v.y);
        };

        // smooth out
        std::for_each(vertices.begin(), vertices.end(), smooth_out);

        // sort

        auto octant = [](const Vec2 & a) -> std::uint_fast32_t
        {
          return
              (static_cast<std::uint_fast32_t>(a.y < 0) << 1) |
              (static_cast<std::uint_fast32_t>(a.y < 0) ^ static_cast<std::uint_fast32_t>(a.x < 0));
        };

        auto atan_comparator = [](Vec2 & a, Vec2 & b)
        {
          return std::atan2(a.y, a.x) < std::atan2(b.y, b.x);
        };

        auto slope_comparator = [octant](const Vec2 & a, const Vec2 & b)
        {
          const auto octant_a = octant(a);
          const auto octant_b = octant(b);

          if (octant_a == octant_b)
              if (octant_a & 1)
                  return a.x * b.y > b.x * a.y;
              else
                  return b.x * a.y < a.x * b.y;
          else
              return octant_a < octant_b;
        };

        std::sort(vertices.begin(), vertices.end(), slope_comparator);

        upload(vertices);
        upload_bb();
    }

    Rock(const Rock & other)
    // FUCK STL! this is needed because it is being called by std::vector when reallocating
    {
//        GLuint m_VAO, m_VBO;//todo
//        GLuint m_VAO_bb, m_VBO_bb;//todo

        m_position      = other.m_position;
        m_velocity      = other.m_velocity;
        m_size          = other.m_size;
        m_vertex_count  = other.m_vertex_count;

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

    void move(float delta_time)
    {
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

    /*void drawe(GLint offset_uniform, GLint size_uniform, GLint col_uniform)
    {
        glUniform2f(offset_uniform, m_position.x, m_position.y);
        glUniform1f(size_uniform, m_size);
        glUniform3f(col_uniform, 1.0f, 1.0f, 1.0f);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_LINE_LOOP, 0, m_vertex_count);
        glBindVertexArray(0);

        glUniform3f(col_uniform, 1.0f, 0.1f, 0.2f);

        glBindVertexArray(m_VAO_bb);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        glBindVertexArray(0);
    }*/

    void draw(GLint offset_uniform, GLint size_uniform, GLint col_uniform, bool draw_bb)
    {
        Vec2 offs[4]{
            { 0.0f, 0.0f },
            { -2.0f, 0.0f },
            { -2.0f, -2.0f },
            { 0.0f, -2.0f }
        };

        // asteroid
        glUniform1f(size_uniform, m_size);
        glUniform3f(col_uniform, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(m_VAO);
        for (const auto & o : offs)
        {
            glUniform2f(offset_uniform, m_position.x + o.x, m_position.y + o.y);
            glDrawArrays(GL_LINE_LOOP, 0, m_vertex_count);
//            break; // TODO: remove . this is just for visualisation ov bb's
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

    AABB boundingBox()
    {
        return AABB{
            Vec2{ -m_size + m_position.x, -m_size  + m_position.y },
            Vec2{  m_size + m_position.x,  m_size  + m_position.y }
        };
    }

    ~Rock()
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
    Vec2 m_position;
    Vec2 m_velocity;

    void upload(const std::vector<Vec2> & polygon)
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *) (0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        glBufferData(GL_ARRAY_BUFFER, polygon.size() * sizeof(polygon[0]), polygon.data(), GL_STATIC_DRAW);
    }

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
