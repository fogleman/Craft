#version 120

uniform sampler2D sampler;
uniform float timer;

varying vec2 fragment_uv;
varying float camera_distance;
varying float fog_factor;
varying float diffuse;

const vec3 fog_color = vec3(0.53, 0.81, 0.92);

void main() {
    vec3 color = vec3(texture2D(sampler, fragment_uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
    vec3 light_color = vec3(0.6);
    vec3 ambient = vec3(0.4);
    if (color == vec3(1.0)) {
        light_color = vec3(0.3);
        ambient = vec3(0.7);
    }
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1.0));
    color = mix(color, fog_color, fog_factor);
    gl_FragColor = vec4(color, 1.0);
}
