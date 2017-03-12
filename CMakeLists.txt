cmake_minimum_required(VERSION 3.6)
project(asteroids)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        ../gl3w/gl3w/build/src/gl3w.c
        main.cpp
        Window.hpp Window.cpp
        Keyboard.hpp Keyboard.cpp
        Shader.hpp Shader.cpp
        Vec2.hpp Vec2.cpp
        AABB.hpp AABB.cpp
        Rock.hpp
        Vec2Gen.hpp
        Ship.hpp
        Projectile.hpp
        Polygon.hpp
        )

add_executable(asteroids ${SOURCE_FILES})


# glfw3
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(asteroids ${GLFW_LIBRARIES})

# gl3w
include_directories(../gl3w/gl3w/build/include)

# dl needed by gl3w ( must be included after gl3w ? )
target_link_libraries(asteroids ${CMAKE_DL_LIBS})