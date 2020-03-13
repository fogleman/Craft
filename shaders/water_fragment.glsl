//From https://github.com/fogleman/Craft/tree/water
#version 120

uniform sampler2D sky_sampler;
uniform float timer;
uniform float daylight;
uniform float fog_distance;
uniform vec3 camera;

varying vec4 point;

const float pi = 3.14159265;

void main() {
    vec3 color = vec3(0.00, 0.33, 0.58);
    vec3 ambient = vec3(daylight * 0.6 + 0.2);
    color = min(color * ambient, vec3(1.0));
    float camera_distance = distance(camera, vec3(point));
    float fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
    float dy = point.y - camera.y;
    float dx = distance(point.xz, camera.xz);
    float fog_height = (atan(dy, dx) + pi / 2) / pi;
    vec3 sky_color = vec3(texture2D(sky_sampler, vec2(timer, fog_height)));
    color = mix(color, sky_color, fog_factor);
    gl_FragColor = vec4(color, 0.7);
}
