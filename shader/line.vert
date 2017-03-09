#version 330 core

layout(location = 0) in vec2 Position;

uniform vec2 offset;
uniform float size;
uniform mat2 rotation;

void main()
{
    vec2 position = rotation * (Position * size) + offset;

    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
}