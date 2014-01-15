#version 120

uniform sampler2D sampler;
uniform bool is_sign;
uniform float alpha;

varying vec2 fragment_uv;

void main() {
    vec4 color = texture2D(sampler, fragment_uv);
    if (is_sign) {
        if (color == vec4(1.0)) {
            discard;
        }
    }
    else {
        color.a = max(color.a, alpha);
    }
    gl_FragColor = color;
}
