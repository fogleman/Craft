#version 420

layout (std140, binding = 1) uniform TextUbo {
  mat4 matrix;
  bool is_sign;
};

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

layout (location = 0) out vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
}
