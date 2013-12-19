#version 120

uniform mat4 matrix;
uniform vec3 camera;

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 fragment_uv;
varying float camera_distance;
varying float fog_factor;
varying float fog_height;
varying float diffuse;

const float pi = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv;
    camera_distance = distance(camera, vec3(position));
    fog_factor = pow(clamp(camera_distance / 192.0, 0.0, 1.0), 4.0);
    fog_height = 2.0 * acos(position.y / 192.0) / pi;
    diffuse = max(0.0, dot(normal, light_direction));
}
