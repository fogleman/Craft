#include "sky_shader.h"
#include "matrix.h"
#include "chunk.h"
#include "cube.h"

namespace konstructs {

    SkyModel::SkyModel(const GLuint _position_attr, const GLuint _uv_attr) :
        position_attr(_position_attr),
        uv_attr(_uv_attr) {
        float *data = new float[12288];
        make_sphere(data, 1, 3);
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 12288 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        delete[] data;
    }

    int SkyModel::vertices() {
        return 512 * 3;
    }

    void SkyModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 8, 0);
        glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    }

    SkyShader::SkyShader(const int _radius, const float _fov, const GLuint _sky_texture,
                         const float _near_distance) :
        ShaderProgram(
        "sky",
        "#version 330\n"
        "uniform mat4 matrix;\n"
        "in vec4 position;\n"
        "in vec2 uv;\n"
        "out vec2 fragment_uv;\n"
        "void main() {\n"
        "    gl_Position = matrix * position;\n"
        "    fragment_uv = uv;\n"
        "}\n",
        "#version 330\n"
        "uniform sampler2D sampler;\n"
        "uniform float timer;\n"
        "in vec2 fragment_uv;\n"
        "out vec4 frag_color;\n"
        "void main() {\n"
        "    vec2 uv = vec2(timer, fragment_uv.t);\n"
        "    frag_color = texture2D(sampler, uv);\n"
        "}\n"),
        position_attr(attributeId("position")),
        uv_attr(attributeId("uv")),
        matrix(uniformId("matrix")),
        sampler(uniformId("sampler")),
        timer(uniformId("timer")),
        radius(_radius),
        fov(_fov),
        sky_texture(_sky_texture),
        near_distance(_near_distance),
        model(position_attr, uv_attr) {}

    void SkyShader::render(const Player &p, const int width, const int height,
                           const float current_timer) {
        bind([&](Context c) {
                c.enable(GL_DEPTH_TEST);
                c.enable(GL_CULL_FACE);
                float aspect_ratio = (float)width / (float)height;
                float max_distance = (radius - 1) * CHUNK_SIZE;
                const Matrix4f m = matrix::projection_perspective(fov, aspect_ratio, near_distance, max_distance) * p.direction();
                c.set(matrix, m);
                c.set(sampler, (int)sky_texture);
                c.set(timer, current_timer);
                c.draw(&model);
                c.disable(GL_CULL_FACE);
                c.disable(GL_DEPTH_TEST);
            });
    }


};
