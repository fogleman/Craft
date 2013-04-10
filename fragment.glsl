#version 330 core

flat in int instance;
out vec3 color;

void main() {
    int i = instance;
    float c = i / 80.0;
    color = vec3(c, c, 1);
}
