#version 420

layout(binding = 0) uniform sampler2D sky_sampler;

layout (std140, binding = 1) uniform SkyUbo {
  mat4 matrix;
  float timer;
};

layout (location = 0) in vec2 fragment_uv;

layout (location = 0) out vec4 frag_color;

void main() {
    vec2 uv = vec2(timer, fragment_uv.t);
    frag_color = texture(sky_sampler, uv);
}
