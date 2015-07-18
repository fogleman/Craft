#version 120

attribute vec3 position;
attribute vec2 uv;

varying vec2 fragment_uv;

void main() {
    gl_Position = vec4(position, 1.0);
    fragment_uv = uv;
}
