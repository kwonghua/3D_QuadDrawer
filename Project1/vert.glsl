#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;

uniform mat4 model;
uniform mat4 view_proj;

out vec3 color;

void main() {
    gl_Position = view_proj * model * vec4(pos, 1.0f);
    color = col;
}
