#version 330 core

uniform mat4 matrix;
uniform vec2 rotation;
uniform vec3 center;
in vec4 position;
in vec2 uv;
in vec3 normal;
out vec2 fragment_uv;
out vec3 fragment_normal;

mat4 identity() {
    return mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

mat4 translate(vec3 offset) {
    return mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        offset.x, offset.y, offset.z, 1
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
    vec2 r = rotation;
    mat4 mv = identity();
    mv = translate(-center) * mv;
    mv = rotate(vec3(cos(r.x), 0, sin(r.x)), r.y) * mv;
    mv = rotate(vec3(0, 1, 0), -r.x) * mv;
    mat4 mvp = matrix * mv;
    gl_Position = mvp * position;

    fragment_uv = uv;
    fragment_normal = normal;
}
