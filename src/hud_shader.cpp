#include "matrix.h"
#include "hud.h"
#include "cube.h"
#include "hud_shader.h"
#include <string>

namespace konstructs {
    using matrix::projection_2d;
    using std::vector;

    Matrix4f hud_translation_matrix(const float scale, const float xscale,
                                    const float screen_area);

    Vector4f hud_offset_vector(const float xscale, const float screen_area);

    void make_block(int type, float x, float y, float z, float size, float *d,
                    const int blocks[256][6]);

    void make_stacks(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                       float *d,
                       const int blocks[256][6]);

    void make_stack_amounts(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks, float *d);

    vector<float> make_square(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background);


    ItemStackModel::ItemStackModel(const GLuint position_attr, const GLuint uv_attr,
                                   const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                                   const int blocks[256][6]) :
        position_attr(position_attr),
        uv_attr(uv_attr) {

        float *data = new float[stacks.size() * 10 * 6 * 6];
        make_stacks(stacks, data, blocks);

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

    AmountModel::AmountModel(const GLuint position_attr, const GLuint uv_attr,
                             const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks):
        position_attr(position_attr),
        uv_attr(uv_attr) {
        int total_text_length = 0;
        for (const auto &pair: stacks) {
            if(pair.second.amount == 0) {
                continue;
            }
            if(pair.second.amount > 9) {
                total_text_length += 2;
            } else {
                total_text_length ++;
            }
        }
        float *data = new float[total_text_length * 7 * 6];

        make_stack_amounts(stacks, data);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, total_text_length * 7 * 6 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        verts = total_text_length * 6;
        delete[] data;
    }

    int AmountModel::vertices() {
        return verts;
    }

    void AmountModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 7, 0);
        glVertexAttribPointer(uv_attr, 4, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 7, (GLvoid *)(sizeof(GLfloat) * 3));
    }

    HudModel::HudModel(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background,
                       const GLuint position_attr, const GLuint uv_attr) :
        position_attr(position_attr),
        uv_attr(uv_attr) {

        auto data = make_square(background);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
                     data.data(), GL_STATIC_DRAW);
        verts = data.size() / 7;
    }

    int HudModel::vertices() {
        return verts;
    }

    void HudModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 7, 0);
        glVertexAttribPointer(uv_attr, 4, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 7, (GLvoid*)(sizeof(GLfloat)*3));
    }

    BlockModel::BlockModel(const GLuint position_attr, const GLuint uv_attr,
                           const int type, const float x, const float y,
                           const float size,
                           const int blocks[256][6]) :
        position_attr(position_attr),
        uv_attr(uv_attr) {
        float *data = new float[10 * 6 * 6];
        make_block(type, x, y, 0.0, size, data, blocks);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, 10 * 6 * 6 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        verts = 6 * 6;
        delete[] data;
    }

    int BlockModel::vertices() {
        return verts;
    }

    void BlockModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, 0);
        glVertexAttribPointer(uv_attr, 4, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    }


    HudShader::HudShader(const int columns, const int rows, const int texture,
                         const int block_texture, const int font_texture) :
        ShaderProgram(
            "hud",

            "#version 330\n"
            "uniform vec4 offset;\n"
            "uniform mat4 matrix;\n"
            "in vec3 position;\n"
            "in vec4 uv;\n"
            "out vec2 fragment_uv;\n"
            "void main() {\n"
            "    fragment_uv = uv.xy;\n"
            "    gl_Position = vec4(position, 1.0) * matrix + offset;\n"
            "}\n",
            "#version 330\n"
            "uniform sampler2D sampler;\n"
            "in vec2 fragment_uv;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    vec3 color = vec3(texture(sampler, fragment_uv));\n"
            "    if (color == vec3(1.0, 0.0, 1.0)) {\n"
            "        discard;\n"
            "    }\n"
            "    fragColor = vec4(color, 1.0);\n"
            "}\n"),
        position(attributeId("position")),
        uv(attributeId("uv")),
        offset(uniformId("offset")),
        matrix(uniformId("matrix")),
        sampler(uniformId("sampler")),
        texture(texture),
        block_texture(block_texture),
        font_texture(font_texture),
        columns(columns),
        rows(rows) {}

    optional<Vector2i> HudShader::clicked_at(const double x, const double y,
                                             const int width, const int height) {
        // Convert to Open GL coordinates (-1 to 1) and inverted y
        double glx = (x / (double)width) * 2.0 - 1.0;
        double gly = (((double)height - y) / (double)height) * 2.0 - 1.0;

        double scale = 4.0/(double)columns;
        double xscale = (double)height / (double)width;

        // Convert to inventory positions
        double ix = (glx + 2.0*xscale*0.6) / (scale*xscale*0.6);
        double iy = (gly + 1.0) / (scale * 0.6);

        // Return position if it is within bounds
        if(ix >= 0.0 && ix < (double)columns && iy >= 0.0 && iy < (double)rows) {
            return optional<Vector2i>({(int)ix, (int)iy});
        } else {
            return nullopt;
        }
    }

    void HudShader::render(const int width, const int height,
                           const float mouse_x, const float mouse_y,
                           const Hud &hud,
                           const int blocks[256][6]) {
        bind([&](Context c) {
                float scale = 4.0f/(float)columns;
                float xscale = (float)height / (float)width;
                float screen_area = 0.6;

                /* Set up for 17 x 14 HUD grid */
                c.set(matrix,  hud_translation_matrix(scale, xscale, screen_area));
                c.set(offset, hud_offset_vector(xscale, screen_area));

                /* Use background texture*/
                c.set(sampler, texture);

                /* Generate and draw background model */
                HudModel hm(hud.backgrounds(), position, uv);
                c.draw(hm);

                /* Use block texture */
                c.set(sampler, block_texture);
                /* Generate and draw item stacks */
                ItemStackModel ism(position, uv, hud.stacks(), blocks);
                c.draw(ism);

                /* Use font texture */
                c.set(sampler, font_texture);

                /* Generate and draw item stack amounts */
                AmountModel am(position, uv, hud.stacks());
                c.draw(am);

                /* Check for held block*/
                auto held = hud.held();
                if(held) {

                    /* Calculate mouse position on screen as gl coordinates */
                    float x = (mouse_x / (float)width) * 2.0f - 1.0f;
                    float y = (((float)height - mouse_y) / (float)height) * 2.0f - 1.0f;

                    /* Set up for drawing on whole screen */
                    Matrix4f m = Matrix4f::Identity();
                    /* This scales items drawn so that they are kept "square" */
                    m(0) = xscale;
                    Vector4f v = Vector4f::Zero();
                    c.set(matrix, m);
                    c.set(offset, v);
                    /* Use block textures */
                    c.set(sampler, block_texture);
                    /* Generate a single block model */
                    BlockModel bm(position, uv, held->type, x / xscale, y,
                                  scale * xscale * 0.6, blocks);
                    c.draw(bm);
                }

            });
    }

    Matrix4f hud_translation_matrix(const float scale, const float xscale,
                                    const float screen_area) {
        Matrix4f m;
        m.col(0) << scale * screen_area * xscale, 0.0f, 0.0f, 0.0f;
        m.col(1) << 0.0f, scale * screen_area, 0.0f, 0.0f;
        m.col(2) << 0.0f, 0.0f, 1.0f, 0.0f;
        m.col(3) << 0.0f, 0.0f, 0.0f, 1.0f;
        return m;
    }

    Vector4f hud_offset_vector(const float xscale, const float screen_area) {
        return Vector4f(-2*xscale*screen_area, -1.0f, 0.0f, 0.0f);
    }

    vector<float> make_square(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background) {
        vector<float> m;
        float ts = 0.25;
        for(auto pair: background) {
            int i = pair.first[0];
            int j = pair.first[1];
            int t = pair.second;
            if(t >= 0) {
                m.push_back(-0.0f+i); m.push_back(1.0f+j); m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);
                m.push_back(1.0f+i);  m.push_back(1.0f+j);  m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);
                m.push_back(-0.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);
                m.push_back(-0.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);
                m.push_back(1.0f+i);  m.push_back(1.0f+j);  m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);
                m.push_back(1.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);
            }
        }
        return m;
    }

    void make_block(int type, float x, float y, float z, float size, float *d,
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
        make_cube(d, ao, light,
                  1, 1, 1, 1, 1, 1,
                  x, y, z, size,
                  type, blocks);
    }

    void make_stacks(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                     float *d,
                     const int blocks[256][6]) {

        int i = 0;
        for (const auto &pair: stacks) {
            make_block(pair.second.type, pair.first[0] + 0.5, pair.first[1] + 0.5, 0, 0.35,
                       d + i * 10 * 6 * 6, blocks);
            i++;
        }
    }

    void make_stack_amounts(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                            float *d) {
        int i = 0;

        for (const auto &pair: stacks) {
            if(pair.second.amount == 0) {
                continue;
            }
            std::string text = std::to_string(pair.second.amount);
            for (int index = 0; index < text.length(); index++) {
                int offset = text.length() - index - 1;
                make_character(d + i * 7 * 6, pair.first[0] - (float)offset*0.3 + 0.75f, pair.first[1] + 0.25f, 0.15, 0.2, text[index], 0.0);
                i++;
            }
        }
    }

};
