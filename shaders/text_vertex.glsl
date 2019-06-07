#version 140

layout (std140) uniform TextUbo {
  mat4 matrix;
  bool is_sign;
};

attribute vec4 position;
attribute vec2 uv;

varying vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
}
