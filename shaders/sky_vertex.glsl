#version 140

layout (std140) uniform SkyUbo {
  mat4 matrix;
  float timer;
};

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
}
