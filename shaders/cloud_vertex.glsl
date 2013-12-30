#version 120

uniform mat4 matrix;
uniform mat4 model;
uniform vec3 camera;

attribute vec4 position;
attribute vec3 normal;
<<<<<<< HEAD
attribute vec3 colour;

varying vec3 fragment_colour;
varying float diffuse;
varying float fog_factor;
=======
attribute vec2 uv;

varying vec2 fragment_uv;
varying float diffuse;
>>>>>>> c0a5776df729aadb57fb2bc851d0c79b620e757e

const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
<<<<<<< HEAD
    vec4 pos = matrix * model * position;
    gl_Position = pos;
    fragment_colour = colour;
    diffuse = max(0.0, dot(normal, light_direction));
    
    float camera_distance = distance(camera, vec3(model * position));
    fog_factor = pow(clamp(camera_distance / 200, 0.0, 1.0), 4.0);
=======
    gl_Position = matrix * model * position;
    fragment_uv = uv;
    
    diffuse = max(0.0, dot(normal, light_direction));
>>>>>>> c0a5776df729aadb57fb2bc851d0c79b620e757e
}
