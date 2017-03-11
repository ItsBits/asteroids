#include "Window.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>

#include "Rock.hpp"
#include "Shader.hpp"
#include "Ship.hpp"
#include "Projectile.hpp"
#include "Polygon.hpp"

int main()
{
    Window window;
    window.makeContextCurrent();

    const auto s = std::chrono::high_resolution_clock::now();

    Shader shader{
        std::vector<Shader::Source>{
            { "shader/line.vert", GL_VERTEX_SHADER },
            { "shader/line.frag", GL_FRAGMENT_SHADER }
        }
    };

    const auto e = std::chrono::high_resolution_clock::now();

    Vec2Gen rng{ (unsigned)(e - s).count() };

    shader.use();

    GLint offset_uniform = glGetUniformLocation(shader.id(), "translation");
    GLint scale_uniform = glGetUniformLocation(shader.id(), "scale");
    GLint col_uniform = glGetUniformLocation(shader.id(), "color");
    GLint rotation_uniform = glGetUniformLocation(shader.id(), "rotation");

    std::vector<Rock> rocks;
    constexpr size_t a_count = 6;

    for(int i = 0; i < a_count; ++i)
        rocks.emplace_back(rng,
            (rng.get().x + 2.0f) / 14.0f, 10,
            (rng.get() * 2.0f - 1.0f), (rng.get() * 2.0f - 1.0f) * 0.3f
        );

    Ship ship{ 0.03f, 1.0f, 1.5f, 0.3f };

    const std::vector<Vec2> aabb
        {
            { -1.0f, -1.0f },
            {  1.0f, -1.0f },
            {  1.0f,  1.0f },
            { -1.0f,  1.0f },
        };

    Polygon aabb_polygon{ aabb };

    std::vector<Projectile> projectiles;

    float time_til_vulneable = 3.0f; // TODO: move that to Ship class

    float score = 0.0f;
    float score_update = 0.0f;
    float delta_time = 0.01f;
    while(!window.exitRequested())
    {
        score -= delta_time * 4.0f;
        score_update -= delta_time;

        window.pollEvents();

        float asteroid_rotation_matrix[4]{ // TODO: generalize translate rotate scale for all objects: http://www.cs.trinity.edu/~jhowland/cs2322/2d/2d/
            1.0f, 0.0f,
            0.0f, 1.0f
        };

        // ship
        ship.move(delta_time);
        const auto shot = ship.shoot(delta_time);

        if (std::get<0>(shot) == true)
        {
            projectiles.emplace_back(std::get<1>(shot));
            score -= 2.0f;
        }

        std::for_each(rocks.begin(), rocks.end(), [delta_time] (Rock & r) { r.move(delta_time); });
        std::for_each(projectiles.begin(), projectiles.end(), [delta_time] (Projectile & p) { p.move(delta_time); });

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (Projectile & p) { return p.isDead(); }), projectiles.end());


        std::vector<Rock> new_rocks;

        for (auto & p : projectiles) // TODO: fix this. it will sometimes miss because of tunneling issue (no swept AABB) or should I call it good enough?
        {
            for (auto i = rocks.begin(); i != rocks.end(); ++i)
            {
                if (AABB::intersect(p.boundingBox(), i->boundingBox())) // broad-phase
                    if (polygons_intersect(p.polygonSRT(), i->polygonSRT())) // narrow-phase
                    {
                        score += 1.0f / (i->size() + 0.01f);

                        const auto new_rock = i->split(rng);
                        if (std::get<0>(new_rock) >= 1) new_rocks.emplace_back(std::get<1>(new_rock));
                        if (std::get<0>(new_rock) == 2) new_rocks.emplace_back(std::get<2>(new_rock));

                        std::cout << new_rocks.size() << std::endl;
                        rocks.erase(i);
                        p.kill();
                        break;
                    }
            }
        }

        rocks.insert(rocks.end(), new_rocks.begin(), new_rocks.end());

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (Projectile & p) { return p.isDead(); }), projectiles.end());

        // check death
        time_til_vulneable -= delta_time;
        if (time_til_vulneable < 0.0f/* && false*/)
        {
            for (auto & p : rocks) // TODO: fix this. it will sometimes miss because of tunneling issue (no swept AABB)
            {
                if (AABB::intersect(ship.boundingBox(), p.boundingBox())) // broad-phase
                    if (polygons_intersect(ship.polygonSRT(), p.polygonSRT())) // narrow-phase
                    {
                        score = -std::numeric_limits<float>::infinity();
                        window.scheduleExit();
                        break;
                    }
            }
        }


        const bool invincible = time_til_vulneable >= 0.0f;
        if (invincible)
            glUniform3f(col_uniform, 0.0f, 1.0f, 0.5f);
        else
            glUniform3f(col_uniform, 1.0f, 0.0f, 0.7f);

        ship.draw(scale_uniform, rotation_uniform, offset_uniform);

        glUniform3f(col_uniform, 0.3f, 0.0f, 1.0f);
        std::for_each(projectiles.begin(), projectiles.end(), [offset_uniform, scale_uniform, rotation_uniform] (Projectile & p) { p.draw(scale_uniform, rotation_uniform, offset_uniform); });

        glUniform3f(col_uniform, 1.0f, 1.0f, 1.0f);
        glUniformMatrix2fv(rotation_uniform, 1, 0, asteroid_rotation_matrix); // TODO: rotate asteroids
        // glUniform3f(col_uniform, 1.0f, 1.0f, 1.0f);
        std::for_each(rocks.begin(), rocks.end(), [offset_uniform, scale_uniform, col_uniform] (Rock & r) { r.draw(offset_uniform, scale_uniform); });

        /*
         * draw bounding boxes
         */

        if (false)
        {
            auto draw_aabb = [offset_uniform, scale_uniform, &aabb_polygon](const auto & element)
            {
                Vec2 position, size;

                position_size_from_AABB(element.boundingBox(), position, size);

                glUniform2f(scale_uniform, size.x, size.y);
                glUniform2f(offset_uniform, position.x, position.y);

                aabb_polygon.draw();
            };

            glUniform3f(col_uniform, 1.0f, 0.0f, 0.0f);

            std::for_each(projectiles.begin(), projectiles.end(), draw_aabb);
            std::for_each(rocks.begin(), rocks.end(), draw_aabb);
            draw_aabb(ship);
        }




        if (score_update < 0.0f)
        {
            score_update = 0.25f;
            std::cout << "Score: " << score << std::endl;
        }
        window.swapResizeClearBuffer();

        // TODO: better timing
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

        { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }

        if (rocks.size() == 0)
            window.scheduleExit();
    }

    std::cout << "Final Score: " << score << std::endl;
}