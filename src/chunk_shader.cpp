#include <iostream>
#include <math.h>
#include "chunk_shader.h"
#include "matrix.h"
#include "util.h"

namespace konstructs {

    const Array3i chunk_offset = Vector3i(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE).array();

    ChunkModel::ChunkModel(const shared_ptr<ChunkModelResult> &data,
                           GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr) :
        position(data->position),
        size(data->size),
        faces(data->faces),
        position_attr(_position_attr),
        normal_attr(_normal_attr),
        uv_attr(_uv_attr) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat),
                     data->data(), GL_STATIC_DRAW);
        Vector3f pos =
            (position.array() * chunk_offset).matrix().cast<float>();

        Vector3f rpos = Vector3f(pos[0], pos[2], pos[1]);
        translation = Affine3f(Translation3f(rpos)).matrix();
    }

    void ChunkModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(normal_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, 0);
        glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 3));
        glVertexAttribPointer(uv_attr, 4, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    }

    void ChunkShader::add(const shared_ptr<ChunkModelResult> &data) {
        auto it = models.find(data->position);
        auto model = new ChunkModel(data, position_attr,
                                    normal_attr, uv_attr);
        if (it != models.end()) {
            auto second = it->second;
            it->second = model;
            delete second;
        } else {
            models.insert({data->position, model});
        }

    }

    void shtxt_path(const char *name, const char *type, char *path, size_t max_len) {
        snprintf(path, max_len, "%s/%s", type, name);

        if (!file_exist(path)) {
            snprintf(path, max_len, "/usr/local/share/konstructs-client/%s/%s", type, name);
        }

        if (!file_exist(path)) {
            printf("Error, no %s for %s found.\n", type, name);
            exit(1);
        }
    }

    void texture_path(const char *name, char *path, size_t max_len) {
        shtxt_path(name, "textures", path, max_len);
    }


    void load_textures() {
        char txtpth[KONSTRUCTS_PATH_SIZE];

        GLuint sky;
        glGenTextures(1, &sky);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sky);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        texture_path("sky.png", txtpth, KONSTRUCTS_PATH_SIZE);
        load_png_texture(txtpth);
    }

    ChunkShader::ChunkShader(const int _radius, const float _fov, const GLuint _block_texture,
                             const GLuint _sky_texture) :
        ShaderProgram(
        "chunk",
        "#version 330\n"
        "uniform mat4 matrix;\n"
        "uniform vec3 camera;"
        "uniform float fog_distance;\n"
        "uniform mat4 translation;\n"
        "in vec4 position;\n"
        "in vec3 normal;\n"
        "in vec4 uv;\n"
        "out vec2 fragment_uv;\n"
        "out float fragment_ao;\n"
        "out float fragment_light;\n"
        "out float fog_factor;\n"
        "out float fog_height;\n"
        "out float diffuse;\n"
        "const float pi = 3.14159265;\n"
        "const vec3 light_direction = normalize(vec3(-1.0, 1.0, -1.0));\n"
        "void main() {\n"
        "    vec4 global_position = translation * position;\n"
        "    gl_Position = matrix * global_position;\n"
        "    fragment_uv = uv.xy;\n"
        "    fragment_ao = 0.3 + (1.0 - uv.z) * 0.7;\n"
        "    fragment_light = uv.w;\n"
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
        "in float fragment_ao;\n"
        "in float fragment_light;\n"
        "in float fog_factor;\n"
        "in float fog_height;\n"
        "in float diffuse;\n"
        "out vec4 frag_color;\n"
        "const float pi = 3.14159265;\n"
        "void main() {\n"
        "    vec3 color = vec3(texture2D(sampler, fragment_uv));\n"
        "    if (color == vec3(1.0, 0.0, 1.0)) {\n"
        "        discard;\n"
        "    }\n"
        "    bool cloud = color == vec3(1.0, 1.0, 1.0);\n"
        "    float df = cloud ? 1.0 - diffuse * 0.2 : diffuse;\n"
        "    float ao = cloud ? 1.0 - (1.0 - fragment_ao) * 0.2 : fragment_ao;\n"
        "    ao = min(1.0, ao + fragment_light);\n"
        "    df = min(1.0, df + fragment_light);\n"
        "    float value = min(1.0, daylight + fragment_light);\n"
        "    vec3 light_color = vec3(value * 0.3 + 0.2);\n"
        "    vec3 ambient = vec3(value * 0.3 + 0.2) + vec3(sin(pi*daylight)/2, sin(pi*daylight)/4, 0.0);\n"
        "    vec3 light = ambient + light_color * df;\n"
        "    color = clamp(color * light * ao, vec3(0.0), vec3(1.0));\n"
        "    vec3 sky_color = vec3(texture2D(sky_sampler, vec2(timer, fog_height)));\n"
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
        radius(_radius),
        fov(_fov),
        block_texture(_block_texture),
        sky_texture(_sky_texture) {
        load_textures();
    }

    int ChunkShader::render(const Player &p, const int width, const int height,
                            const float current_daylight, const float current_timer) {
        int faces = 0;
        int visible = 0;
        bind([&](Context c) {
                c.enable(GL_DEPTH_TEST);
                c.enable(GL_CULL_FACE);
                float aspect_ratio = (float)width / (float)height;
                float max_distance = (radius - 1) * CHUNK_SIZE;
                const Matrix4f m = matrix::projection_perspective(fov, aspect_ratio, 0.25, max_distance) * p.view();
                c.set(matrix, m);
                c.set(sampler, (int)block_texture);
                c.set(sky_sampler, (int)sky_texture);
                c.set(fog_distance, max_distance);
                c.set(daylight, current_daylight);
                c.set(timer, current_timer);
                c.set(camera, p.camera());
                float planes[6][4];
                matrix::ext_frustum_planes(planes, radius, m);
                for(const auto &pair : models) {
                    const auto &m = pair.second;
                    if(!chunk_visible(planes, m->position))
                        continue;
                    visible++;
                    c.set(translation, m->translation);
                    c.draw(m, 0, m->faces*6);
                    faces += m->faces;
                }
                c.disable(GL_CULL_FACE);
                c.disable(GL_DEPTH_TEST);
            });
        std::cout << "CHUNKS: " << models.size() << " VISIBLE: " << visible << std::endl;
        return faces;
    }

    bool chunk_visible(const float planes[6][4], const Vector3i &position) {
        float x = position[0] * CHUNK_SIZE - 1;
        float z = position[1] * CHUNK_SIZE - 1;
        float y = position[2] * CHUNK_SIZE - 1;
        float d = CHUNK_SIZE + 1;
        float points[8][3] = {
            {x + 0, y + 0, z + 0},
            {x + d, y + 0, z + 0},
            {x + 0, y + 0, z + d},
            {x + d, y + 0, z + d},
            {x + 0, y + d, z + 0},
            {x + d, y + d, z + 0},
            {x + 0, y + d, z + d},
            {x + d, y + d, z + d}
        };
        for (int i = 0; i < 6; i++) {
            int in = 0;
            int out = 0;
            for (int j = 0; j < 8; j++) {
                float d =
                    planes[i][0] * points[j][0] +
                    planes[i][1] * points[j][1] +
                    planes[i][2] * points[j][2] +
                    planes[i][3];
                if (d < 0) {
                    out++;
                }
                else {
                    in++;
                }
                if (in && out) {
                    break;
                }
            }
            if (in == 0) {
                return false;
            }
        }
        return true;
    }

};
