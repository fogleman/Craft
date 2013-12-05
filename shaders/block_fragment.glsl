#version 330 core

uniform sampler2D sampler;
uniform float timer;

in vec2 fragment_uv;
flat in float camera_distance;
flat in float fog_factor;
flat in float diffuse;

out vec3 color;

const vec3 light_color = vec3(0.6);
const vec3 ambient = vec3(0.4);
const vec3 fog_color = vec3(0.53, 0.81, 0.92);

void main() {
    color = vec3(texture(sampler, fragment_uv));
    if (color == vec3(1, 0, 1)) {
        discard;
    }

    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1));
    color = mix(color, fog_color, fog_factor);
}
