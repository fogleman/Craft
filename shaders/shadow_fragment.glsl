#version 330 core

out float depth;

void main() {
    depth = gl_FragCoord.z;
}
