#version 330 core

uniform mat4 matrix;
uniform float timer;
in vec4 position;
in vec2 uv;
out vec2 fragment_uv;

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
    int i = gl_InstanceID;
    int z = i / 81;
    int y = (i % 81) / 9 - 4;
    int x = (i % 81) % 9 - 4;

    float m = 2;
    vec4 p = position;
    //p = rotate(vec3(0, 1, 0), timer) * p;
    p = translate(vec3(x * m, y * m, -z * m - 5)) * p;
    //p = translate(vec3(0, -10, 0)) * p;
    p = rotate(vec3(1, 0, 0), sin(timer) / 4) * p;
    p = matrix * p;
    gl_Position = p;

    int t = i % 4;
    fragment_uv = uv + vec2(t * 0.125, 0);
}
