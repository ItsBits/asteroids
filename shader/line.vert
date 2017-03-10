#version 330 core

layout(location = 0) in vec2 Position;

uniform vec2 scale;
uniform mat2 rotation;
uniform vec2 translation;

void main()
{
    gl_Position = vec4(vec2((rotation * (Position * scale)) + translation), 0.0f, 1.0f);
}