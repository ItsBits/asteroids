#include "Window.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>

#include "Rock.hpp"
#include "Shader.hpp"
#include "Ship.hpp"
#include "Projectile.hpp"

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

    GLint offset_uniform = glGetUniformLocation(shader.id(), "offset");
    GLint count_uniform = glGetUniformLocation(shader.id(), "size");
    GLint col_uniform = glGetUniformLocation(shader.id(), "col");
    GLint rotation_uniform = glGetUniformLocation(shader.id(), "rotation");

    std::vector<Rock> rocks;
    constexpr size_t a_count = 6;

    for(int i = 0; i < a_count; ++i)
        rocks.emplace_back(
            (rng.get().x + 2.0f) / 14.0f, 10, rng,
            rng.get(), rng.get(), 0.6f
        );

    Ship ship{ 0.03f, 0.5f };

    std::vector<Projectile> projectiles;

    float time_til_vulneable = 3.0f;

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
        ship.move(delta_time, 1.5f, 1.0f);
        bool shot = ship.shoot(projectiles, rng, delta_time, 0.3f);

        if (shot)
            score -= 2.0f;

        std::for_each(rocks.begin(), rocks.end(), [delta_time] (Rock & r) { r.move(delta_time); });
        std::for_each(projectiles.begin(), projectiles.end(), [delta_time] (Projectile & p) { p.move(delta_time); });

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (Projectile & p) { return p.dead(); }), projectiles.end());

        Vec2 offs[]{
            { -2.0f, -2.0f },
            {  0.0f, -2.0f },
            {  2.0f, -2.0f },
            { -2.0f,  0.0f },
            {  0.0f,  0.0f },
            {  2.0f,  0.0f },
            { -2.0f,  2.0f },
            {  0.0f,  2.0f },
            {  2.0f,  2.0f }
        };

        std::vector<Rock> new_rocks;

        for (auto & p : projectiles) // TODO: fix this. it will sometimes miss because of tunneling issue (no swept AABB)
        {
            for (auto i = rocks.begin(); i != rocks.end(); ++i)
            {
                auto breakk = false;

                for (auto & of : offs) // TODO: isn't there a faster way?????!!!!
                    if (AABB::intersect(p.boundingBox() + of, i->boundingBox()))
                    {
                        score += 1.0f / (i->size() + 0.01f);
                        new_rocks.push_back(i->split(rng));
                        new_rocks.push_back(i->split(rng));
                        rocks.erase(i);
                        p.markDead();
                        breakk = true;
                        break;
                    }

                if (breakk) break;
            }
        }

        // TODO: add new_rocks
        std::for_each(new_rocks.begin(), new_rocks.end(), [&rocks](Rock & r){ if (r.size() > 0.08f) rocks.push_back(r); });

        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (Projectile & p) { return p.dead(); }), projectiles.end());

        // check death
        time_til_vulneable -= delta_time;
        if (time_til_vulneable < 0.0f)
        {
            for (auto & p : rocks) // TODO: fix this. it will sometimes miss because of tunneling issue (no swept AABB)
            {
                if (AABB::intersect(ship.boundingBox(), p.boundingBox()))
                {
                    score = -std::numeric_limits<float>::infinity();
                    window.scheduleExit();
                    break;
                }
            }
        }

        ship.draw(offset_uniform, count_uniform, col_uniform, rotation_uniform, time_til_vulneable >= 0.0f, true);
        glUniformMatrix2fv(rotation_uniform, 1, 0, asteroid_rotation_matrix);
        std::for_each(rocks.begin(), rocks.end(), [offset_uniform, count_uniform, col_uniform] (Rock & r) { r.draw(offset_uniform, count_uniform, col_uniform, true); });
        std::for_each(projectiles.begin(), projectiles.end(), [offset_uniform, count_uniform, col_uniform] (Projectile & p) { p.draw(offset_uniform, count_uniform, col_uniform); });

        if (score_update < 0.0f)
        {
            score_update = 0.25f;
            std::cout << "Score: " << score << std::endl;
        }
        window.swapResizeClearBuffer();
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

        { const GLenum r = glGetError(); assert(r == GL_NO_ERROR); }

        if (rocks.size() == 0)
            window.scheduleExit();
    }

    std::cout << "Final Score: " << score << std::endl;
}