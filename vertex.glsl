#version 330 core

uniform mat4 matrix;
uniform float timer;
in vec4 position;
in vec2 uv;
out vec2 fragment_uv;
flat out int instance;

mat4 translate(vec3 t) {
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0
    );
}

mat4 rotate(vec3 axis, float angle) {
    vec3 a = normalize(axis);
    float x = a.x;
    float y = a.y;
    float z = a.z;
    float s = sin(angle);
    float c = cos(angle);
    float m = 1.0 - c;
    return mat4(
        m * x * x + c, m * x * y - z * s, m * z * x + y * s, 0.0,
        m * x * y + z * s, m * y * y + c, m * y * z - x * s, 0.0,
        m * z * x - y * s, m * y * z + x * s, m * z * z + c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main() {
    fragment_uv = uv;
    instance = gl_InstanceID;
    int x = instance % 9 - 4;
    int y = instance / 9 - 4;

    vec4 p = position;
    p = rotate(vec3(0, 1, 0), timer) * p;
    p = translate(vec3(x * 2, y * 2, -12)) * p;
    p = matrix * p;
    gl_Position = p;
}
