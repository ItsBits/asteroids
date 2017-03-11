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
    Rock() {}

    //==========================================================================
    Rock(Vec2Gen & rng, float size, int vertex_count, Vec2 position, Vec2 velocity) :
        m_size{ std::max(0.0f, size) },
        m_position{ position },
        m_velocity{ velocity }
    {
        vertex_count = std::max(4, vertex_count);

        std::vector<Vec2> vertices;

        vertices.reserve(static_cast<std::size_t>(vertex_count));

        // generate normalized polar coordinates of vertices
        for (int i = 0; i < vertex_count; ++i)
            vertices.push_back(rng.get());

        assert(vertices.size() >= 4);

        // guarantee that at leas one point is inside of each quadrant
        for (int i = 0; i < 4; ++i)
            vertices[i].y = vertices[i].y * 0.25f + static_cast<float>(i) * 0.25f;

        // sort by polar angle
        std::sort(vertices.begin(), vertices.end(),
                  [](const Vec2 & a, const Vec2 & b) { return a.y < b.y; }
        );

        // convert normalized polar to cartesian coordinates
        std::for_each(vertices.begin(), vertices.end(),
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

        m_polygon.update(vertices);

        // calculate symmetric AABB
        std::vector<Vec2> polygon = m_polygon.vertices();
        // scale
        std::for_each(polygon.begin(), polygon.end(), [this](Vec2 & v){ v = v * m_size; });

        const Vec2 size2 = AABB_to_size(compute_ABB_from_polygon(polygon));
        m_bounding_box = AABB{ { -size2.x, -size2.y }, size2 }; // symmetric AABB

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
        m_size    { other.m_size },
        m_position{ other.m_position },
        m_velocity{ other.m_velocity },
        m_polygon { std::move(other.m_polygon) },
        m_bounding_box { other.m_bounding_box }
    {
        other.m_size     = 0;
        other.m_position = Vec2{ 0.0f, 0.0f };
        other.m_velocity = Vec2{ 0.0f, 0.0f };
        other.m_bounding_box = AABB{ Vec2{ 0.0f, 0.0f }, Vec2{ 0.0f, 0.0f } };
    }

    //==========================================================================
    Rock & operator = (Rock && other)
    {
        m_size     = other.m_size;
        m_position = other.m_position;
        m_velocity = other.m_velocity;

        m_polygon = std::move(other.m_polygon);
        m_bounding_box = other.m_bounding_box;

        other.m_size     = 0;
        other.m_position = Vec2{ 0.0f, 0.0f };
        other.m_velocity = Vec2{ 0.0f, 0.0f };

        other.m_bounding_box = AABB{ Vec2{ 0.0f, 0.0f }, Vec2{ 0.0f, 0.0f } };

        return *this;
    }

    //==========================================================================
    Rock(const Rock & other) = default;

    //==========================================================================
    ~Rock() = default;

    //==========================================================================
    void move(float delta_time)
    {
        // euler integration
        m_position.x += m_velocity.x * delta_time;
        m_position.y += m_velocity.y * delta_time;

        // wrap/warp around
        wrap_around(m_position, { m_size, m_size });
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
    void draw(GLint translation_uniform, GLint scale_uniform)
    {
        glUniform2f(scale_uniform, m_size, m_size);
        glUniform2f(translation_uniform, m_position.x, m_position.y);

        m_polygon.draw();
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
            result.emplace_back(v * m_size + m_position);

        return result;
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

    std::tuple<int, Rock, Rock> split(Vec2Gen & rng)
    {
        // TODO: actually split on vertices that are hit by the projectile
        // TODO: when splitting kill if split blocks are smaller than 4 vertices
        // aka. improve splitting

        const auto size = m_polygon.size();

        if (size < 5)
            return std::make_tuple(0, Rock{}, Rock{});
        else if (size < 8)
            return std::make_tuple(
                1,
                Rock{ rng, m_size / 1.5f, m_polygon.size() / 2, m_position, (rng.get() * 2.0f - 1.0f) * 0.15f },
                Rock{}
            );
        else
            return std::make_tuple(
                2,
                Rock{ rng, m_size / 1.5f, m_polygon.size() / 2, m_position, (rng.get() * 2.0f - 1.0f) * 0.15f },
                Rock{ rng, m_size / 1.5f, m_polygon.size() / 2, m_position, (rng.get() * 2.0f - 1.0f) * 0.15f }
            );
    }




private:
    float m_size; // TODO: class Object (vec2 size, vec2 position, vec2 velocity) and reuse it in all other similar classes (ship rock projectile)
    Vec2 m_position;
    Vec2 m_velocity;

    Polygon m_polygon;

    AABB m_bounding_box;

};
