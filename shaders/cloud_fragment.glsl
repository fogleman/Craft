#version 120

uniform vec3 cloudColour;
uniform sampler2D sky_sampler;
uniform float timer;

varying vec3 fragment_colour;
varying float diffuse;
varying float fog_factor;

void main() {
    vec3 color = fragment_colour;

    vec3 light_color = vec3(0.6);
    vec3 ambient = vec3(0.4);
    if (color == vec3(1.0)) {
        light_color = vec3(0.3);
        ambient = vec3(0.7);
    }
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1.0));
    
    color = color * cloudColour;
    vec3 sky_color = vec3(texture2D(sky_sampler, vec2(timer, 0.55)));
    color = mix(color, sky_color, fog_factor);
    gl_FragColor = vec4(color, 1.0);
}
