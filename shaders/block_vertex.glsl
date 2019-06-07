#version 420

layout (std140, binding = 2) uniform BlockUbo {
  mat4 matrix;
  vec3 camera;
  float timer;
  float daylight;
  float fog_distance;
  bool ortho;
};

layout (location = 0) in vec4 position;
layout (location = 2) in vec4 uv;
layout (location = 1) in vec3 normal;

layout (location = 0) out vec2 fragment_uv;
layout (location = 1) out float fragment_ao;
layout (location = 2) out float fragment_light;
layout (location = 3) out float fog_factor;
layout (location = 4) out float fog_height;
layout (location = 5) out float diffuse;

const float pi = 3.14159265;
const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));

void main() {
    gl_Position = matrix * position;
    fragment_uv = uv.xy;
    fragment_ao = 0.3 + (1.0 - uv.z) * 0.7;
    fragment_light = uv.w;
    diffuse = max(0.0, dot(normal, light_direction));
    if (ortho) {
        fog_factor = 0.0;
        fog_height = 0.0;
    }
    else {
        float camera_distance = distance(camera, vec3(position));
        fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
        float dy = position.y - camera.y;
        float dx = distance(position.xz, camera.xz);
        fog_height = (atan(dy, dx) + pi / 2) / pi;
    }
}
