#version 120

uniform mat4 matrix;

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec4 point;

void main() {
    gl_Position = matrix * position;
    point = position;
}
