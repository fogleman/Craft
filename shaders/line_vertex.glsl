#version 420

layout (std140, binding = 0) uniform LineUbo {
  mat4 matrix;
};

layout (location = 0) in vec4 position;

void main() {
    gl_Position = matrix * position;
}
