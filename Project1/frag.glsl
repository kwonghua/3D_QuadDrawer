#version 330 core

uniform mat4 model;
uniform mat4 view_proj;

in vec3 color;

void main() {
    gl_FragColor = vec4(color, 1.0f);
}
