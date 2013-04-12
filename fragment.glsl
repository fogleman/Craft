#version 330 core

uniform sampler2D sampler;
in vec2 fragment_uv;
out vec4 color;

void main() {
    vec4 fog = vec4(0.53, 0.81, 0.92, 1.00);
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float f = clamp(z / 60, 0, 1);
    f  = pow(f, 2);
    color = texture(sampler, fragment_uv);
    color = mix(color, fog, f);
}
