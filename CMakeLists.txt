cmake_minimum_required(VERSION 3.6)
project(asteroids)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES
        main.cpp
        Window.cpp
        Window.hpp
        Keyboard.hpp
        ../gl3w/gl3w/build/src/gl3w.c
        Keyboard.cpp Rock.hpp Vec2.hpp
        Shader.cpp Shader.hpp Vec2Gen.hpp AABB.hpp Ship.hpp Projectile.hpp)
add_executable(asteroids ${SOURCE_FILES})


# glfw3
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(asteroids ${GLFW_LIBRARIES})

# gl3w
include_directories(../gl3w/gl3w/build/include)

# dl needed by gl3w ( must be included after gl3w ? )
target_link_libraries(asteroids ${CMAKE_DL_LIBS})