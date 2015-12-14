#include "matrix.h"
#include "hud_shader.h"

namespace konstructs {
    using matrix::projection_2d;

    MatrixXf make_square(const int columns, const int rows, const int type);

    HudModel::HudModel(const int *background_buffer,
                       GLuint _position_attr, GLuint color_attr,
                       const int columns, const int rows) :
        position_attr(_position_attr),
        color_attr(color_attr) {

        auto data = make_square(columns, rows, 0);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
                     data.data(), GL_STATIC_DRAW);

    }


    void HudModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(color_attr);
        glVertexAttribPointer(position_attr, 2, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 3, 0);
        glVertexAttribPointer(color_attr, 1, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 3, (GLvoid*)(sizeof(GLfloat)*2));
    }

    HudShader::HudShader(int columns, int rows) :
        ShaderProgram(
            "hud",

            "#version 330\n"
            "uniform float scale;\n"
            "uniform float xscale;\n"
            "in vec2 position;\n"
            "in float color;\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            "    out_color = vec4(color, 0.0f, 0.0f, 1.0f);\n"
            "    vec4 pos = vec4(position.x*scale * 0.6 * xscale, position.y*scale*0.6 - 1.0, 0, 1);\n"
            "    gl_Position = vec4(pos.x - 2*xscale*0.6, pos.y, pos.z, pos.w);\n"
            "}\n",

            "#version 330\n"
            "in vec4 out_color;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    fragColor = out_color;\n"
            "}\n"),
        position(attributeId("position")),
        color(attributeId("color")),
        scale(uniformId("scale")),
        xscale(uniformId("xscale")),
        columns(columns),
        rows(rows){
    }

    void HudShader::render(const int width, const int height) {
        bind([&](Context c) {
                HudModel m(NULL, position, color, columns, rows);
                c.set(scale, 4.0f/(float)columns);
                c.set(xscale, (float)height / (float)width);
                c.draw(m, 0, columns*rows*6);
            });
    }

    MatrixXf make_square(const int columns, const int rows, const int type) {
        MatrixXf m(3, columns*rows*6);
        for (int j=0; j<rows; j++) {
            for (int i=0; i<columns; i++) {
                int k = 6*i+j*columns*6;
                m.col(k + 0) << -0.0f+i,  1.0f+j, 0.02f * (float)i * (float)j + 0.2f;
                m.col(k + 1) <<  1.0f+i,  1.0f+j, 0.02f * (float)i * (float)j + 0.2f;
                m.col(k + 2) << -0.0f+i, -0.0f+j, 0.02f * (float)i * (float)j + 0.2f;
                m.col(k + 3) << -0.0f+i, -0.0f+j, 0.02f * (float)i * (float)j + 0.2f;
                m.col(k + 4) <<  1.0f+i,  1.0f+j, 0.02f * (float)i * (float)j + 0.2f;
                m.col(k + 5) <<  1.0f+i, -0.0f+j, 0.02f * (float)i * (float)j + 0.2f;
            }
        }
        return m;
    }
};
