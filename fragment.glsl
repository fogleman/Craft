#version 330 core

uniform sampler2D sampler;
in vec2 fragment_uv;
in vec3 fragment_normal;
out vec3 color;

void main() {
    color = vec3(texture(sampler, fragment_uv));

    vec3 light_direction = normalize(vec3(1, -1, 1));
    float diffuse = max(0, dot(fragment_normal, light_direction));
    vec3 ambient = vec3(0.4);
    vec3 light_color = vec3(1);
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1));

    vec3 fog = vec3(0.53, 0.81, 0.92);
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float f = clamp(z / 128, 0, 1);
    f = pow(f, 2);
    color = mix(color, fog, f);
}
