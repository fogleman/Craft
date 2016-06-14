#version 330
uniform sampler2D sampler;
uniform sampler2D sky_sampler;
uniform float timer;
uniform vec3 light_color;
uniform vec3 ambient_light;

in vec2 fragment_uv;
in float fragment_ao;
in float fog_factor;
in float fog_height;
in float diffuse;

out vec4 frag_color;

void main() {
    vec3 color = vec3(texture(sampler, fragment_uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
    vec3 light = ambient_light + light_color * diffuse;
    color = clamp(color * light * fragment_ao, vec3(0.0), vec3(1.0));
    vec3 sky_color = vec3(texture(sky_sampler, vec2(timer, fog_height)));
    color = mix(color, sky_color, fog_factor);
    frag_color = vec4(color, 1.0);
}
