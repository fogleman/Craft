#version 120

<<<<<<< HEAD
uniform vec3 cloudColour;
uniform sampler2D sky_sampler;
uniform float timer;

varying vec3 fragment_colour;
varying float diffuse;
varying float fog_factor;

void main() {
    vec3 color = fragment_colour;

=======
uniform sampler2D sampler;
uniform float timer;

varying vec2 fragment_uv;
varying float diffuse;

const vec3 fog_color = vec3(0.53, 0.81, 0.92);

void main() {
    vec3 color = vec3(texture2D(sampler, fragment_uv));
    if (color == vec3(1.0, 0.0, 1.0)) {
        discard;
    }
>>>>>>> c0a5776df729aadb57fb2bc851d0c79b620e757e
    vec3 light_color = vec3(0.6);
    vec3 ambient = vec3(0.4);
    if (color == vec3(1.0)) {
        light_color = vec3(0.3);
        ambient = vec3(0.7);
    }
    vec3 light = ambient + light_color * diffuse;
    color = min(color * light, vec3(1.0));
<<<<<<< HEAD
    
    color = color * cloudColour;
    vec3 sky_color = vec3(texture2D(sky_sampler, vec2(timer, 0.55)));
    color = mix(color, sky_color, fog_factor);
=======
>>>>>>> c0a5776df729aadb57fb2bc851d0c79b620e757e
    gl_FragColor = vec4(color, 1.0);
}
