#version 120

uniform mat4 matrix;
uniform mat4 model;
uniform vec3 camera;

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 fragment_uv;
varying float diffuse;

const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
    gl_Position = matrix * model * position;
    fragment_uv = uv;
    
    diffuse = max(0.0, dot(normal, light_direction));
}
