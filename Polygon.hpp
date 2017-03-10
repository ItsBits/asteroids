#pragma once

#include <GL/gl3w.h>

#include <vector>
#include <cassert>

#include "Vec2.hpp"

class Polygon
{
public:
    //==========================================================================
    Polygon() {}

    //==========================================================================
    Polygon(const std::vector<Vec2> & vertices)
    {
        update(vertices);
    }

    //==========================================================================
    Polygon (const Polygon & other)
    {
        if (other.m_VAO == 0)
            return;

        m_vertex_count = other.m_vertex_count;

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *)0);
        glEnableVertexAttribArray(0);

        glBufferData(GL_ARRAY_BUFFER, m_vertex_count * sizeof(Vec2), nullptr, GL_STATIC_DRAW);

        glBindBuffer(GL_COPY_READ_BUFFER, other.m_VBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, m_vertex_count * sizeof(Vec2));
    }

    //==========================================================================
    Polygon(Polygon && other)
    {
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_vertex_count = other.m_vertex_count;

        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_vertex_count = 0;
    }

    //==========================================================================
    Polygon & operator = (const Polygon & other)
    {
        Polygon tmp{ other };
        *this = std::move(tmp);
        return *this;
    }

    //==========================================================================
    Polygon & operator = (Polygon && other)
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);

        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_vertex_count  = other.m_vertex_count;

        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_vertex_count = 0;

        return *this;
    }

    //==========================================================================
    ~Polygon()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);

        m_VAO = 0;
        m_VBO = 0;
        m_vertex_count = 0;
    }

    //==========================================================================
    void update(const std::vector<Vec2> & vertices)
    {
        if (vertices.size() == 0)
        {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);

            m_VAO = 0;
            m_VBO = 0;
            m_vertex_count = 0;

            return;
        }

        if (m_VAO == 0)
        {
            assert(m_VBO == 0);

            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);

            glBindVertexArray(m_VAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (GLvoid *)0);
            glEnableVertexAttribArray(0);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        }

        m_vertex_count = static_cast<GLsizei>(vertices.size());

        glBufferData(GL_ARRAY_BUFFER, m_vertex_count * sizeof(Vec2), vertices.data(), GL_STATIC_DRAW);
    }

    //==========================================================================
    void draw()
    {
        if (m_VAO == 0)
        {
            assert(m_VBO == 0);
            return;
        }

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_LINE_LOOP, 0, m_vertex_count);
    }

    //==========================================================================
    GLsizei size() const
    {
        return m_vertex_count;
    }

private:
    GLuint m_VAO{ 0 };
    GLuint m_VBO{ 0 };
    GLsizei m_vertex_count{ 0 };

};