#version 330 core

uniform mat4 matrix;
uniform vec3 camera;

in vec4 position;
in vec3 normal;

flat out float diffuse;
flat out float camera_distance;
flat out float fog_factor;

const vec3 light_direction = normalize(vec3(-1, -1, -1));

void main() {
    gl_Position = matrix * position;

    camera_distance = distance(camera, vec3(position));
    fog_factor = pow(clamp(camera_distance / 80, 0, 1), 4);
    diffuse = max(0, dot(normal, light_direction));
}
