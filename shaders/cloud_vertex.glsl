#version 120

uniform mat4 matrix;
uniform mat4 model;
uniform vec3 camera;

attribute vec4 position;
attribute vec3 normal;
attribute vec3 colour;

varying vec3 fragment_colour;
varying float diffuse;
varying float fog_factor;

const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
    vec4 pos = matrix * model * position;
    gl_Position = pos;
    fragment_colour = colour;
    diffuse = max(0.0, dot(normal, light_direction));
    
    float camera_distance = distance(camera, vec3(model * position));
    fog_factor = pow(clamp(camera_distance / 200, 0.0, 1.0), 4.0);
}
