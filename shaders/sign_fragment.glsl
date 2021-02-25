#version 450
layout(binding = 0) uniform sampler2D sign_sampler;

layout(location = 0) in vec2 fragment_uv;

layout(location = 0) out vec4 frag_color;

void main() {
    vec4 color = texture(sign_sampler, fragment_uv);
    if (color == vec4(1.0)) {
        discard;
    }
    frag_color = color;
}
