#version 120

uniform sampler2D sampler;
uniform bool shaded;

varying vec2 fragment_uv;

void main() {
    vec4 color = texture2D(sampler, fragment_uv);
    if (shaded) {
        color.a = max(color.a, 0.4);
    }
    gl_FragColor = color;
}
