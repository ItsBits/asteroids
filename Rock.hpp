#pragma once

#include <vector>
#include <algorithm>

#include "Vec2.hpp"
#include "Vec2Gen.hpp"
#include "AABB.hpp"
#include "Polygon.hpp"

#include <GL/gl3w.h>


class Rock
{
public:
    //==========================================================================
    Rock(Vec2Gen & rng, float size, int vertex_count, Vec2 position, Vec2 velocity) :
        m_size{ std::max(0.0f, size) },
        m_position{ position },
        m_velocity{ velocity }
    {
        vertex_count = std::max(4, vertex_count);

        m_vertices.reserve(static_cast<std::size_t>(vertex_count));

        // generate normalized polar coordinates of vertices
        for (int i = 0; i < vertex_count; ++i)
            m_vertices.push_back(rng.get());

        assert(m_vertices.size() >= 4);

        // guarantee that at leas one point is inside of each quadrant
        for (int i = 0; i < 4; ++i)
            m_vertices[i].y = m_vertices[i].y * 0.25f + static_cast<float>(i) * 0.25f;

        // sort by polar angle
        std::sort(m_vertices.begin(), m_vertices.end(),
                  [](const Vec2 & a, const Vec2 & b) { return a.y < b.y; }
        );

        // convert normalized polar to cartesian coordinates
        std::for_each(m_vertices.begin(), m_vertices.end(),
                      [](Vec2 & v)
                      {
                          // * hardcoded size interpretation
                          // * angle scale should be a little less than 2 x pi
                          v = {
                              (v.x * 0.3f + 0.7f) * std::cos(v.y * 6.283f),
                              (v.x * 0.3f + 0.7f) * std::sin(v.y * 6.283f)
                          };
                      }
        );

        m_polygon.update(m_vertices);
    }

    //==========================================================================
    Rock & operator = (const Rock & other)
    {
        Rock tmp{ other };
        *this = std::move(tmp);
        return *this;
    }

    //==========================================================================
    Rock(Rock && other) :
        m_size{ other.m_size },
        m_position{ other.m_position },
        m_velocity{ other.m_velocity },
        m_vertices{ std::move(other.m_vertices) },
        m_polygon{ std::move(other.m_polygon) }
    {
        other.m_size = 0;
        other.m_position = Vec2{ 0.0f, 0.0f };
        other.m_velocity = Vec2{ 0.0f, 0.0f };
    }

    //==========================================================================
    Rock & operator = (Rock && other)
    {
        m_size = other.m_size;
        m_position = other.m_position;
        m_velocity = other.m_velocity;

        m_vertices = std::move(other.m_vertices);
        m_polygon = std::move(other.m_polygon);

        other.m_size = 0;
        other.m_position = Vec2{ 0.0f, 0.0f };
        other.m_velocity = Vec2{ 0.0f, 0.0f };

        return *this;
    }

    //==========================================================================
    Rock(const Rock & other) = default;

    //==========================================================================
    ~Rock() = default;

    //==========================================================================
    void move(float delta_time) // TODO: reuse tis function for all objects
    {
        // euler integration
        m_position.x += m_velocity.x * delta_time;
        m_position.y += m_velocity.y * delta_time;

        // wrap/warp around
        if (m_position.x < -1.0f - m_size) m_position.x += 2.0f + 2.0f * m_size;
        if (m_position.y < -1.0f - m_size) m_position.y += 2.0f + 2.0f * m_size;
        if (m_position.x >  1.0f + m_size) m_position.x -= 2.0f + 2.0f * m_size;
        if (m_position.y >  1.0f + m_size) m_position.y -= 2.0f + 2.0f * m_size;
    }

    //==========================================================================
    AABB boundingBox()
    {
        return {
            { m_position.x - m_size, m_position.y - m_size },
            { m_position.x + m_size, m_position.y + m_size }
        };
    }

    //==========================================================================
    void draw(GLint translation_uniform, GLint scale_uniform)
    {
        glUniform2f(scale_uniform, m_size, m_size);
        glUniform2f(translation_uniform, m_position.x, m_position.y);

        m_polygon.draw();
    }



    /*
     *
     *
     *
     *
     *
     * refactor stuff below
     */

    float size() const { return m_size; } // TODO: when splitting kill if split blocks are smaller than 4 vertices

    std::size_t vertexCount() const { return m_vertices.size(); }

    Rock split(Vec2Gen & rng)
    {
        // TODO: actually split on vertices that are hit by the projectile
        // TODO: when splitting kill if split blocks are smaller than 4 vertices
        return Rock{
            rng, m_size / 2.0f, m_polygon.size() / 2, m_position, rng.get()
        };
    }




private:
    float m_size;
    Vec2 m_position;
    Vec2 m_velocity;

    std::vector<Vec2> m_vertices;
    Polygon m_polygon;

};
