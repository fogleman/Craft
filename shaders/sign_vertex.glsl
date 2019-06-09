#version 450

layout (std140, binding = 1) uniform MatUbo {
  mat4 matrix;
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
}
