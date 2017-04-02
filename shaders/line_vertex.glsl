#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 matrix;

attribute vec4 position;

void main() {
    gl_Position = matrix * position;
}
