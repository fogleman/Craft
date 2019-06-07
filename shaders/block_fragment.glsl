#version 420

layout(binding = 0) uniform sampler2D block_sampler;
layout(binding = 1) uniform sampler2D sky_sampler;

layout (std140, binding = 2) uniform BlockUbo {
  mat4 matrix;
  vec3 camera;
  float timer;
  float daylight;
  float fog_distance;
  bool ortho;
};

layout (location = 0) in vec2 fragment_uv;
layout (location = 1) in float fragment_ao;
layout (location = 2) in float fragment_light;
layout (location = 3) in float fog_factor;
layout (location = 4) in float fog_height;
layout (location = 5) in float diffuse;

const float pi = 3.14159265;

layout (location = 0) out vec4 frag_color;

void main() {
    vec3 color = vec3(texture(block_sampler, fragment_uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
    bool cloud = color == vec3(1.0, 1.0, 1.0);
    if (cloud && ortho) {
        discard;
    }
    float df = cloud ? 1.0 - diffuse * 0.2 : diffuse;
    float ao = cloud ? 1.0 - (1.0 - fragment_ao) * 0.2 : fragment_ao;
    ao = min(1.0, ao + fragment_light);
    df = min(1.0, df + fragment_light);
    float value = min(1.0, daylight + fragment_light);
    vec3 light_color = vec3(value * 0.3 + 0.2);
    vec3 ambient = vec3(value * 0.3 + 0.2);
    vec3 light = ambient + light_color * df;
    color = clamp(color * light * ao, vec3(0.0), vec3(1.0));
    vec3 sky_color = vec3(texture(sky_sampler, vec2(timer, fog_height)));
    color = mix(color, sky_color, fog_factor);
    frag_color = vec4(color, 1.0);
}
