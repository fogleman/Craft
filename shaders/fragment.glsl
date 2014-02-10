#version 330 core

out vec3 color;

in float diffuse;
flat in float camera_distance;
flat in float fog_factor;

const vec3 light_color = vec3(0.4);
const vec3 fog_color = vec3(0.53, 0.81, 0.92);
const vec3 ambient = vec3(0.6);

void main() {
    color = vec3(0.8, 0.9, 1.0);
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1));
    color = mix(color, fog_color, fog_factor);
}
