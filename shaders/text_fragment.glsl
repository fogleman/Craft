#version 420

layout(binding = 0) uniform sampler2D text_sampler;

layout (std140, binding = 1) uniform TextUbo {
  mat4 matrix;
  bool is_sign;
};

layout (location = 0) in vec2 fragment_uv;

layout (location = 0) out vec4 frag_color;

void main() {
    vec4 color = texture(text_sampler, fragment_uv);
    if (is_sign) {
        if (color == vec4(1.0)) {
            discard;
        }
    }
    else {
        color.a = max(color.a, 0.4);
    }
    frag_color = color;
}
