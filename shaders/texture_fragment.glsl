#version 120

uniform sampler2D sampler;

varying vec2 fragment_uv;

void main() {
    vec4 color = texture2D(sampler, fragment_uv);
    if (color == vec4(1.0, 0.0, 1.0, 1.0)) {
        discard;
    }
    gl_FragColor = vec4(color.rgb, 0.5);
}
