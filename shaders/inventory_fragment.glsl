#version 120

uniform sampler2D sampler;

varying vec2 fragment_uv;

void main() {
    vec4 color = texture2D(sampler, fragment_uv);
    gl_FragColor = color;
}
