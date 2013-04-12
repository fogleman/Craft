#version 330 core

uniform sampler2D sampler;
in vec2 fragment_uv;
out vec4 color;

void main() {
    vec4 fog = vec4(0, 0, 0, 1);
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float f = clamp(z / 30, 0, 1);
    color = texture(sampler, fragment_uv);
    color = mix(color, fog, f);
}
