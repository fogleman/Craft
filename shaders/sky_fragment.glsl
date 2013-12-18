#version 120

uniform sampler2D sampler;
uniform float timer;

varying vec2 fragment_uv;

void main() {
    gl_FragColor = texture2D(sampler, fragment_uv);
}
