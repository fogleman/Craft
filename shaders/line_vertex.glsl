#version 140

layout (std140) uniform LineUbo {
  mat4 matrix;
};

attribute vec4 position;

void main() {
    gl_Position = matrix * position;
}
