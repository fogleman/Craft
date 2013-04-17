#version 330 core

uniform mat4 matrix;

in vec4 position;
in vec3 normal;
in vec2 uv;

out vec4 fragment_position;
out vec3 fragment_normal;
out vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_position = position;
    fragment_normal = normal;
    fragment_uv = uv;
}
