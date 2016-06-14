#include <iostream>
#include <math.h>
#include "player_shader.h"
#include "matrix.h"

namespace konstructs {

    void PlayerShader::add(const Player player) {
        players.erase(player.id);
        players.insert({player.id, player});
    }

    void PlayerShader::remove(const int pid) {
        players.erase(pid);
    }


    PlayerShader::PlayerShader(const float fov, const GLuint player_texture,
                               const GLuint sky_texture, const float near_distance,
                               tinyobj::shape_t &shape) :
        ShaderProgram(
        "player",
        "#version 330\n"
        "uniform mat4 matrix;\n"
        "uniform vec3 camera;\n"
        "uniform float fog_distance;\n"
        "uniform mat4 translation;\n"
        "in vec4 position;\n"
        "in vec3 normal;\n"
        "in vec2 uv;\n"
        "out vec2 fragment_uv;\n"
        "out float fog_factor;\n"
        "out float fog_height;\n"
        "out float diffuse;\n"
        "const float pi = 3.14159265;\n"
        "const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));\n"
        "void main() {\n"
        "    vec4 global_position = translation * position;\n"
        "    gl_Position = matrix * global_position;\n"
        "    fragment_uv = uv.xy;\n"
        "    diffuse = max(0.0, dot(normal, light_direction));\n"
        "    float camera_distance = distance(camera, vec3(global_position));\n"
        "    fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);\n"
        "    float dy = global_position.y - camera.y;\n"
        "    float dx = distance(global_position.xz, camera.xz);\n"
        "    fog_height = (atan(dy, dx) + pi / 2) / pi;\n"
        "}\n",
        "#version 330\n"
        "uniform sampler2D sampler;\n"
        "uniform sampler2D sky_sampler;\n"
        "uniform float timer;\n"
        "uniform float daylight;\n"
        "in vec2 fragment_uv;\n"
        "in float fog_factor;\n"
        "in float fog_height;\n"
        "in float diffuse;\n"
        "out vec4 frag_color;\n"
        "const float pi = 3.14159265;\n"
        "void main() {\n"
        "    vec3 color = vec3(texture(sampler, fragment_uv));\n"
        "    if (color == vec3(1.0, 0.0, 1.0)) {\n"
        "        discard;\n"
        "    }\n"
        "    float df = diffuse;\n"
        "    float value = min(1.0, daylight);\n"
        "    vec3 light_color = vec3(value * 0.3 + 0.2);\n"
        "    vec3 ambient = vec3(value * 0.3 + 0.2) + vec3(sin(pi*daylight)/2, sin(pi*daylight)/4, 0.0);\n"
        "    vec3 light = ambient + light_color * df;\n"
        "    color = clamp(color * light, vec3(0.0), vec3(1.0));\n"
        "    vec3 sky_color = vec3(texture(sky_sampler, vec2(timer, fog_height)));\n"
        "    color = mix(color, sky_color, fog_factor);\n"
        "    frag_color = vec4(color, 1.0);\n"
        "}\n"),
        position_attr(attributeId("position")),
        normal_attr(attributeId("normal")),
        uv_attr(attributeId("uv")),
        matrix(uniformId("matrix")),
        translation(uniformId("translation")),
        sampler(uniformId("sampler")),
        sky_sampler(uniformId("sky_sampler")),
        fog_distance(uniformId("fog_distance")),
        timer(uniformId("timer")),
        daylight(uniformId("daylight")),
        camera(uniformId("camera")),
        fov(fov),
        player_texture(player_texture),
        sky_texture(sky_texture),
        near_distance(near_distance),
        model(position_attr, normal_attr, uv_attr, shape)
    {}

    int PlayerShader::render(const Player &p, const int width, const int height,
                             const float current_daylight, const float current_timer, const int radius) {
        int faces = 0;
        int visible = 0;
        bind([&](Context c) {
                c.enable(GL_DEPTH_TEST);
                c.enable(GL_CULL_FACE);
                float aspect_ratio = (float)width / (float)height;
                float max_distance = (radius - 1) * CHUNK_SIZE;
                const Matrix4f m = matrix::projection_perspective(fov, aspect_ratio, near_distance, max_distance) * p.view();
                c.set(matrix, m);
                c.set(sampler, (int)player_texture);
                c.set(sky_sampler, (int)sky_texture);
                c.set(fog_distance, max_distance);
                c.set(daylight, current_daylight);
                c.set(timer, current_timer);
                c.set(camera, p.camera());
                for(const auto &pair: players) {
                    const auto &p = pair.second;
                    c.set(translation, p.translation());
                    c.draw(model);
                }
                c.disable(GL_CULL_FACE);
                c.disable(GL_DEPTH_TEST);
            });
        return faces;
    }


};
