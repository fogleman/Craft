#version 420

layout (std140, binding = 1) uniform SkyUbo {
  mat4 matrix;
  float timer;
};

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 fragment_uv;

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
}
