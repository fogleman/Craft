#version 330 core

uniform sampler2D sampler;
uniform sampler2D shadow_map;
uniform float timer;

in vec2 fragment_uv;
in vec4 shadow_coord;
flat in float camera_distance;
flat in float fog_factor;
flat in float diffuse;

out vec3 color;

const vec3 light_color = vec3(0.6);
const vec3 ambient = vec3(0.4);
const vec3 fog_color = vec3(0.53, 0.81, 0.92);

float lookup(vec4 coord, vec2 offset) {
    float bias = 0.0001;
    float z = texture(shadow_map, coord.xy + offset / 4096).r;
    return float(z > coord.z - bias);
}

vec2 random_offset(vec4 seed) {
    float angle = fract(sin(dot(seed, vec4(12.9898, 78.233, 45.164, 94.673)))
        * 43758.5453) * radians(360.0);
    return vec2(cos(angle), sin(angle)) * 2;
}

void main() {
    color = vec3(texture(sampler, fragment_uv));

    float visibility = 1;
    if (camera_distance < 16) {
        float sum = 0;
        int count = 4;
        for (int i = 0; i < count; i++) {
            vec2 offset = random_offset(vec4(gl_FragCoord.xyz, i));
            sum += lookup(shadow_coord, offset);
        }
        visibility = 0.5 + sum / count / 2;
    }

    vec3 light = (ambient + light_color * diffuse) * visibility;
    color = min(color * light, vec3(1));
    color = mix(color, fog_color, fog_factor);
}
