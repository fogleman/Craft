#version 330 core

uniform sampler2D sampler;
uniform float timer;
uniform vec3 camera;

flat in vec3 fragment_position;
flat in vec3 fragment_normal;
in vec2 fragment_uv;

out vec3 color;

const vec3 light_direction = normalize(vec3(1, 0, 1));
const vec3 light_color = vec3(1);
const vec3 ambient = vec3(0.4);
const vec3 fog_color = vec3(0.53, 0.81, 0.92);

void main() {
    color = vec3(texture(sampler, fragment_uv));

    float diffuse = max(0, dot(fragment_normal, light_direction));
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1));

    float z = distance(camera, fragment_position);
    float f = clamp(z / 192, 0, 1);
    f = pow(f, 4);
    color = mix(color, fog_color, f);
}
