#version 140

uniform sampler2D sampler;

layout (std140) uniform SkyUbo {
  mat4 matrix;
  float timer;
};

varying vec2 fragment_uv;

void main() {
    vec2 uv = vec2(timer, fragment_uv.t);
    gl_FragColor = texture2D(sampler, uv);
}
