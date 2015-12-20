#include "matrix.h"
#include "cube.h"
#include "item_shader.h"

namespace konstructs {
    using std::vector;
    float* make_stacks(const int columns, const int rows,
                       const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const int blocks[256][6]);

    ItemStackModel::ItemStackModel(const GLuint position_attr, const GLuint uv_attr,
                                   const int columns, const int rows,
                                   const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                                   const int blocks[256][6]) :
        position_attr(position_attr),
        uv_attr(uv_attr) {

        auto data = make_stacks(columns, rows, stacks, blocks);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, stacks.size() * 10 * 6 * 6 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        verts = stacks.size() * 6 * 6;
        delete[] data;
    }

    int ItemStackModel::vertices() {
        return verts;
    }

    void ItemStackModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, 0);
        glVertexAttribPointer(uv_attr, 4, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    }

    ItemShader::ItemShader(const int columns, const int rows, const int texture) :
        ShaderProgram(
            "item",

            "#version 330\n"
            "uniform float scale;\n"
            "uniform float xscale;\n"
            "in vec3 position;\n"
            "in vec4 uv;\n"
            "out vec2 fragment_uv;\n"
            "void main() {\n"
            "    fragment_uv = uv.xy;\n"
            "    vec4 pos = vec4(position.x*scale * 0.6 * xscale, position.y*scale*0.6 - 1.0, position.z, 1);\n"
            "    gl_Position = vec4(pos.x - 2*xscale*0.6, pos.y, pos.z, pos.w);\n"
            "}\n",

            "#version 330\n"
            "uniform sampler2D sampler;\n"
            "in vec2 fragment_uv;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    vec3 color = vec3(texture2D(sampler, fragment_uv));\n"
            "    if (color == vec3(1.0, 0.0, 1.0)) {\n"
            "        discard;\n"
            "    }\n"
            "    fragColor = vec4(color, 1.0);\n"
            "}\n"),
        position(attributeId("position")),
        uv(attributeId("uv")),
        scale(uniformId("scale")),
        xscale(uniformId("xscale")),
        sampler(uniformId("sampler")),
        texture(texture),
        columns(columns),
        rows(rows){
    }

    void ItemShader::render(const int width, const int height,
                            const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                            const int blocks[256][6]) {
        bind([&](Context c) {
                ItemStackModel m(position, uv, columns, rows, stacks, blocks);
                c.set(scale, 4.0f/(float)columns);
                c.set(xscale, (float)height / (float)width);
                c.set(sampler, texture);
                c.draw(m);
            });
    }

    float* make_stacks(const int columns, const int rows,
                       const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       const int blocks[256][6]) {
        float ao[6][4] = {0};
        float light[6][4] = {
            {0.5, 0.5, 0.5, 0.5},
            {0.5, 0.5, 0.5, 0.5},
            {0.5, 0.5, 0.5, 0.5},
            {0.5, 0.5, 0.5, 0.5},
            {0.5, 0.5, 0.5, 0.5},
            {0.5, 0.5, 0.5, 0.5}
        };

        int i = 0;
        float *d = new float[stacks.size() * 10 * 6 * 6];
        for (const auto &pair: stacks) {
            make_cube(d + i * 10 * 6 * 6, ao, light,
                      1, 1, 1, 1, 1, 1,
                      pair.first[0] + 0.5, pair.first[1] + 0.5, 0, 0.35,
                      pair.second.type, blocks);
            i++;
        }
        return d;
    }
};
