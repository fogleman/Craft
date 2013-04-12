#version 330 core

uniform isamplerBuffer world;
uniform mat4 matrix;
uniform float timer;
uniform vec2 rotation;
uniform vec3 center;
in vec4 position;
in vec2 uv;
out vec2 fragment_uv;

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
    ivec4 data = texelFetch(world, i);

    vec4 p = position;
    vec2 r = rotation;
    p = p + vec4(data.x, data.y, data.z, 0);
    p = p + vec4(center, 0);
    p = rotate(vec3(cos(r.x), 0, sin(r.x)), r.y) * p;
    p = rotate(vec3(0, 1, 0), -r.x) * p;
    p = matrix * p;
    gl_Position = p;

    fragment_uv = uv + vec2(data.w * 0.125, 0);
}
