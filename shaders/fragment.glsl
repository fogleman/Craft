#version 330 core

out vec3 color;

flat in float diffuse;
flat in float camera_distance;
flat in float fog_factor;

const vec3 light_color = vec3(0.6);
const vec3 fog_color = vec3(0);
const vec3 ambient = vec3(0.4);

void main() {
    color = vec3(0, 0.5, 0.25);
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1));
    color = mix(color, fog_color, fog_factor);
}
