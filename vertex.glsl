#version 330 core

uniform mat4 matrix;
uniform float timer;
in vec4 position;
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
    vec4 p = position;
    int i = gl_InstanceID;
    instance = i;
    int x = i % 9 - 4;
    int y = i / 9 - 4;
    p = rotate(vec3(0, 1, 0), timer * 4) * p;
    p = translate(vec3(sin(timer) * 3 + x * 2, y * 2, cos(timer) * 10 - 20)) * p;
    p = matrix * p;
    gl_Position = p;
}
