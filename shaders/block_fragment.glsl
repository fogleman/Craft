#version 120

uniform sampler2D sampler;
uniform sampler2D sampler2;
uniform float timer;

varying vec2 fragment_uv;
varying float camera_distance;
varying float fog_factor;
varying float fog_height;
varying float diffuse;

const float pi = 3.14159265;

void main() {
    vec3 color = vec3(texture2D(sampler, fragment_uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
    vec3 fog_color = vec3(texture2D(sampler2, vec2(timer, fog_height)));
    float p = sin(timer * 2.0 * pi - pi / 2.0) / 2.0 + 0.5;
    vec3 light_color = vec3(p * 0.5);
    vec3 ambient = vec3(p * 0.3 + 0.2);
    if (color == vec3(1.0)) {
        ambient += p / 4.0;
    }
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1.0));
    color = mix(color, fog_color, fog_factor);
    gl_FragColor = vec4(color, 1.0);
}
