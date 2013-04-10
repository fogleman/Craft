#version 330 core

uniform sampler2D sampler;
flat in int instance;
in vec2 UV;
out vec3 color;

void main() {
    color = vec3(UV, 1);
    //color = texture(sampler, UV).rgb;
}
