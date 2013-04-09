#version 330 core

uniform mat4 matrix;
uniform float timer;
in vec4 position;

mat4 rotationMatrix(vec3 axis, float angle) {
    vec3 a = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(
        oc * a.x * a.x + c,
        oc * a.x * a.y - a.z * s,
        oc * a.z * a.x + a.y * s,
        0.0,
        oc * a.x * a.y + a.z * s,
        oc * a.y * a.y + c,
        oc * a.y * a.z - a.x * s,
        0.0,
        oc * a.z * a.x - a.y * s,
        oc * a.y * a.z + a.x * s,
        oc * a.z * a.z + c,
        0.0,
        0.0,
        0.0,
        0.0,
        1.0
    );
}

void main() {
    gl_Position = matrix * rotationMatrix(vec3(0, 1, 0), sin(timer * 2) / 2) * position;
}
