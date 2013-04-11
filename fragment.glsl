#version 330 core

uniform sampler2D sampler;
flat in int instance;
in vec2 fragment_uv;
out vec4 color;

void main() {
    color = texture(sampler, fragment_uv);
}
