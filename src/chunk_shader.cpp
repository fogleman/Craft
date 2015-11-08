#include <iostream>
#include <thread>
#include "chunk_shader.h"
#include "matrix.h"

namespace konstructs {

    ChunkModel::ChunkModel(Vector3f _position, float *data, size_t size,
                           GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr) :
        position(_position),
        size(size),
        position_attr(_position_attr),
        normal_attr(_normal_attr),
        uv_attr(_uv_attr) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, size * 10  * sizeof(GLfloat), data, GL_STATIC_DRAW);
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

    Matrix4f ChunkModel::translation() {
        return Affine3f(Translation3f(position)).matrix();
    }

    void ChunkShader::add(Vector3f position, float *data, size_t size) {
        models.push_back(new ChunkModel(position, data, size, position_attr,
                                     normal_attr, uv_attr));
    }

    ChunkShader::ChunkShader() :
        ShaderProgram(
        "chunk",
        "#version 330\n"
        "uniform mat4 matrix;\n"
        "uniform mat4 translation;\n"
        "uniform mat4 view;\n"
        "in vec4 position;\n"
        "in vec3 normal;\n"
        "in vec4 uv;\n"
        "out vec4 c;\n"
        "void main() {\n"
        "    gl_Position = matrix * view * translation * position;\n"
        "    c = vec4(1.0, 1.0, normal.x + uv.x, 1.0);\n"
        "}",
        "#version 330\n"
        "in vec4 c;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = c;\n"
        "}"),
        position_attr(attributeId("position")),
        normal_attr(attributeId("normal")),
        uv_attr(attributeId("uv")),
        matrix(uniformId("matrix")),
        translation(uniformId("translation")),
        view(uniformId("view"))
    {}

    void ChunkShader::render(const Player &p, const int width, const int height) {
        bind([&](Context c) {
                c.set(matrix, matrix::projection(width, height));
                c.set(view, p.view());
                for(auto m : models) {
                    c.set(translation, m->translation());
                    c.render(m, 0, m->size);
                }
            });
    }


    ChunkModelFactory::ChunkModelFactory() {
        for(int i = 0; i < WORKERS; i++) {
            new std::thread(&ChunkModelFactory::worker, this);
        }
    }

    void ChunkModelFactory::create_model(std::shared_ptr<ChunkData> data) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            chunks.push(data);
        }
        chunks_condition.notify_one();
    }

    std::vector<std::shared_ptr<ChunkModel>> ChunkModelFactory::fetch_models() {
        std::vector<std::shared_ptr<ChunkModel>> return_models;
        {
            std::lock_guard<std::mutex> lock(mutex);
            return_models = models;
            models.clear();
        }
        return return_models;
    }

    void ChunkModelFactory::worker() {
        std::cout << "WORKER STARTED" << std::endl;
        while(1) {
            std::unique_lock<std::mutex> lk(mutex);
            chunks_condition.wait(lk, [&]{return !chunks.empty();});
            auto data = chunks.front();
            chunks.pop();
            lk.unlock();
            std::cout << "WORKER PROCESSING CHUNK" << std::endl;
        }
    }
};


/*
          "#version 330\n"
        "uniform mat4 matrix;\n"
        "uniform vec3 camera;\n"
        "uniform float fog_distance;\n"
        "uniform mat4 translation;\n"

        "attribute vec4 position;\n"
        "attribute vec3 normal;\n"
        "attribute vec4 uv;\n"

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
        "   }\n"
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
        "}\n"

        camera(uniformId("camera")),
        fog_distance(uniformId("fog_distance")),
        translation(uniformId("translation")),
        timer(uniformId("timer")),
        daylight(uniformId("daylight"))

 */
