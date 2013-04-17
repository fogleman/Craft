#version 330 core

uniform mat4 matrix;

in vec4 position;
in vec3 normal;
in vec2 uv;

flat out vec3 fragment_position;
flat out vec3 fragment_normal;
out vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_position = vec3(position);
    fragment_normal = normal;
    fragment_uv = uv;
}
