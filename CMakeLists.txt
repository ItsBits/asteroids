cmake_minimum_required(VERSION 3.6)
project(asteroids)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        ../gl3w/gl3w/build/src/gl3w.c
        src/main.cpp
        src/Window.hpp src/Window.cpp
        src/Keyboard.hpp src/Keyboard.cpp
        src/Shader.hpp src/Shader.cpp
        src/Vec2.hpp src/Vec2.cpp
        src/AABB.hpp src/AABB.cpp
        src/Rock.hpp
        src/Vec2Gen.hpp
        src/Ship.hpp
        src/Projectile.hpp
        src/Polygon.hpp
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