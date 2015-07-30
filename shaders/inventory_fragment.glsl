#version 120

uniform sampler2D sampler;

varying vec2 fragment_uv;

void main() {
    gl_FragColor = texture2D(sampler, fragment_uv);
}
