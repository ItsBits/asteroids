#include "Window.hpp"

#include <chrono>
#include <thread>
#include <cassert>

#include "Shader.hpp"
#include "Rock.hpp"
#include "Ship.hpp"

constexpr bool DRAW_AABB = false;

constexpr float SHIP_SIZE = 0.04f;

static const std::vector<Shader::Source> SHADER_SOURCE
{
    { "shader/line.vert", GL_VERTEX_SHADER },
    { "shader/line.frag", GL_FRAGMENT_SHADER }
};

static const std::vector<Vec2> AABB_MODEL
{
    { -1.0f, -1.0f },
    {  1.0f, -1.0f },
    {  1.0f,  1.0f },
    { -1.0f,  1.0f }
};

static constexpr std::chrono::microseconds delta_time_ms{ 15'000ul };

static constexpr float identity_matrix[4]
{
    1.0f, 0.0f,
    0.0f, 1.0f
};

int main()
{
    // window
    Window window;
    window.makeContextCurrent();

    // shader
    Shader shader{ SHADER_SOURCE };

    shader.use();

    const GLint translation_uniform = glGetUniformLocation(shader.id(), "translation");
    const GLint scale_uniform       = glGetUniformLocation(shader.id(), "scale");
    const GLint color_uniform       = glGetUniformLocation(shader.id(), "color");
    const GLint rotation_uniform    = glGetUniformLocation(shader.id(), "rotation");

    // random number generator
    const auto seed = std::chrono::duration_cast<std::chrono::nanoseconds>
        (
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();

    Vec2Gen rng{ static_cast<uint32_t>(seed) };

    // rocks
    std::vector<Rock> rocks;

    for(std::size_t i = 0; i < 6; ++i)
        rocks.emplace_back(rng,
            (rng.get().x + 2.0f) / 14.0f, 10,
            (rng.get() * 2.0f - 1.0f), (rng.get() * 2.0f - 1.0f) * 0.15f
        );

    // ship
    Ship ship{ SHIP_SIZE, 0.5f, 0.8f, 0.6f }; // TODO: figure out why collision with ship is not correct

    // projectiles
    std::vector<Projectile> projectiles;

    // axis aligned bounding box
    Polygon aabb_polygon{ AABB_MODEL };

    float invincibility_left = 3.0f;

    float delta_time = static_cast<float>(static_cast<double>(delta_time_ms.count()) / 1'000'000.0);

    while(!window.exitRequested())
    {
        auto start_time = std::chrono::steady_clock::now();

        window.pollEvents();

        // move ship and shoot
        ship.move(delta_time);
        const auto shot = ship.shoot(delta_time);
        if (std::get<0>(shot) == true)
            projectiles.emplace_back(std::get<1>(shot));

        // move rocks
        std::for_each(rocks.begin(), rocks.end(), [delta_time] (Rock & r) { r.move(delta_time); });
        // move projectiles
        std::for_each(projectiles.begin(), projectiles.end(), [delta_time] (Projectile & p) { p.move(delta_time); });

        // remove projectiles that reached end of life
        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (const Projectile & p) { return p.isDead(); }), projectiles.end());

        // perform projectile-rock collision detection and resolution
        std::vector<Rock> new_rocks;

        // used algorithm can miss collisions due to tunneling
        for (auto & p : projectiles)
            for (auto i = rocks.begin(); i != rocks.end(); ++i)
                if (AABB::intersect(p.boundingBox(), i->boundingBox())) // broad-phase
                    if (polygons_intersect(p.polygonSRT(), i->polygonSRT())) // narrow-phase
                    {
                        // split hit rock
                        const auto new_rock = i->split(rng);

                        if (std::get<0>(new_rock) >= 1) new_rocks.emplace_back(std::get<1>(new_rock));
                        if (std::get<0>(new_rock) == 2) new_rocks.emplace_back(std::get<2>(new_rock));

                        rocks.erase(i);

                        // mark used projectile dead
                        p.kill();

                        // projectile can only hit one rock (this also prevents using invalidated iterators)
                        break;
                    }

        // insert new rocks
        rocks.insert(rocks.end(), new_rocks.begin(), new_rocks.end());

        // remove projectiles that have hit rocks
        projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [] (Projectile & p) { return p.isDead(); }), projectiles.end());

        // perform ship-rock collision detection and resolution
        invincibility_left -= delta_time;
        if (invincibility_left < 0.0f)
            // used algorithm can miss collisions due to tunneling
            for (auto & p : rocks)
                if (AABB::intersect(ship.boundingBox(), p.boundingBox())) // broad-phase
                    if (polygons_intersect(ship.polygonSRT(), p.polygonSRT())) // narrow-phase
                    {
                        window.scheduleExit();
                        break;
                    }

        // draw ship
        if (invincibility_left >= 0.0f)
            glUniform3f(color_uniform, 0.0f, 1.0f, 0.5f);
        else
            glUniform3f(color_uniform, 1.0f, 0.0f, 0.7f);

        ship.draw(scale_uniform, rotation_uniform, translation_uniform);

        // draw projectiles
        glUniform3f(color_uniform, 0.6f, 0.5f, 1.0f);
        std::for_each(projectiles.begin(), projectiles.end(), [scale_uniform, rotation_uniform, translation_uniform] (Projectile & p) { p.draw(scale_uniform, rotation_uniform, translation_uniform); });

        // draw rocks
        glUniform3f(color_uniform, 1.0f, 1.0f, 1.0f);
        glUniformMatrix2fv(rotation_uniform, 1, 0, identity_matrix);
        std::for_each(rocks.begin(), rocks.end(), [translation_uniform, scale_uniform] (const Rock & r) { r.draw(translation_uniform, scale_uniform); });

        // draw bounding boxes
        if (DRAW_AABB)
        {
            auto draw_aabb = [translation_uniform, scale_uniform, &aabb_polygon](const auto & element)
            {
                Vec2 position, size;

                position_size_from_AABB(element.boundingBox(), position, size);

                glUniform2f(scale_uniform, size.x, size.y);
                glUniform2f(translation_uniform, position.x, position.y);

                aabb_polygon.draw();
            };

            glUniform3f(color_uniform, 1.0f, 0.0f, 0.0f);

            std::for_each(projectiles.begin(), projectiles.end(), draw_aabb);
            std::for_each(rocks.begin(), rocks.end(), draw_aabb);
            draw_aabb(ship);
        }

        start_time += delta_time_ms;
        std::this_thread::sleep_until(start_time);

        window.swapResizeClearBuffer();

        if (rocks.size() == 0)
            window.scheduleExit();

        {
            const GLenum r = glGetError();
            assert(r == GL_NO_ERROR);
        }
    }
}
