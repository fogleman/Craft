#include <string>
#include <math.h>
#include "matrix.h"
#include "hud.h"
#include "cube.h"
#include "hud_shader.h"

namespace konstructs {
    using matrix::projection_2d;
    using std::vector;

    Matrix4f hud_translation_matrix(const float scale, const float xscale,
                                    const float screen_area);

    Vector4f hud_offset_vector(const float xscale, const float screen_area);

    void make_block(int type, float x, float y, float z, float size,
                    float rx, float ry, float rz, float *d,
                    const BlockTypeInfo &blocks);

    void make_stacks(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                     float *d,
                     const float rx,
                     const float ry,
                     const float rz,
                     const BlockTypeInfo &blocks);

    void make_stack_amounts(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks, float *d);

    vector<float> make_square(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background);

    BaseModel::BaseModel(const GLuint position_attr, const GLuint normal_attr,
                         const GLuint uv_attr) :
        position_attr(position_attr), normal_attr(normal_attr),
        uv_attr(uv_attr) {}

    void BaseModel::bind() {
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

    int BaseModel::vertices() {
        return verts;
    }

    ItemStackModel::ItemStackModel(const GLuint position_attr, const GLuint normal_attr,
                                   const GLuint uv_attr,
                                   const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                                   const BlockTypeInfo &blocks) :
        BaseModel(position_attr, normal_attr, uv_attr) {

        verts = 0;
        for (const auto &pair: stacks) {
            if(blocks.is_plant[pair.second.type])
                verts += 6;
            else
                verts += (6 * 6);
        }

        float *data = new float[verts * 10];
        make_stacks(stacks, data, - M_PI / 8, M_PI / 8, 0, blocks);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, verts * 10 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        delete[] data;
    }

    AmountModel::AmountModel(const GLuint position_attr, const GLuint normal_attr,
                             const GLuint uv_attr,
                             const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks):
        BaseModel(position_attr, normal_attr, uv_attr) {
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
        float *data = new float[total_text_length * 10 * 6];

        make_stack_amounts(stacks, data);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, total_text_length * 10 * 6 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        verts = total_text_length * 6;
        delete[] data;
    }

    HudModel::HudModel(const std::unordered_map<Vector2i, int, matrix_hash<Vector2i>> &background,
                       const GLuint position_attr, const GLuint normal_attr,
                       const GLuint uv_attr) :
        BaseModel(position_attr, normal_attr, uv_attr) {

        auto data = make_square(background);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
                     data.data(), GL_STATIC_DRAW);
        verts = data.size() / 10;
    }

    BlockModel::BlockModel(const GLuint position_attr, const GLuint normal_attr,
                           const GLuint uv_attr,
                           const int type, const float x, const float y,
                           const float size,
                           const BlockTypeInfo &blocks) :
        BaseModel(position_attr, normal_attr, uv_attr) {
        verts = blocks.is_plant[type] ? 6 : 6 * 6;
        float *data = new float[verts * 10];
        make_block(type, x, y, 0.0, size, - M_PI / 8, M_PI / 8, M_PI / 32, data, blocks);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, verts * 10 * sizeof(GLfloat),
                     data, GL_STATIC_DRAW);
        delete[] data;
    }

    HudShader::HudShader(const int columns, const int rows, const int texture,
                         const int block_texture, const int font_texture) :
        ShaderProgram(
            "hud",

            "#version 330\n"
            "uniform vec4 offset;\n"
            "uniform mat4 matrix;\n"
            "in vec3 position;\n"
            "in vec3 normal;\n"
            "in vec4 uv;\n"
            "out vec2 fragment_uv;\n"
            "out float diffuse;\n"
            "const vec3 light_direction = normalize(vec3(1.0, 0.5, 0.0));\n"
            "void main() {\n"
            "    fragment_uv = uv.xy;\n"
            "    diffuse = max(0.0, dot(normal, light_direction));\n"
            "    gl_Position = vec4(position, 1.0) * matrix + offset;\n"
            "}\n",
            "#version 330\n"
            "uniform sampler2D sampler;\n"
            "in vec2 fragment_uv;\n"
            "in float diffuse;\n"
            "out vec4 fragColor;\n"
            "const vec3 light_color = vec3(1.0, 1.0, 1.0);\n"
            "void main() {\n"
            "    vec4 color = vec4(texture(sampler, fragment_uv));\n"
            "    if (color.xyz == vec3(1.0, 0.0, 1.0)) {\n"
            "        discard;\n"
            "    }\n"
            "    float fr_a = max(color.a, 0.7f);\n"
            "    fragColor = vec4(mix(color.xyz, light_color * diffuse, 0.2), fr_a);\n"
            "}\n"),
        position(attributeId("position")),
        normal(attributeId("normal")),
        uv(attributeId("uv")),
        offset(uniformId("offset")),
        matrix(uniformId("matrix")),
        sampler(uniformId("sampler")),
        texture(texture),
        block_texture(block_texture),
        font_texture(font_texture),
        columns(columns),
        rows(rows),
        screen_area(0.6) {}

    optional<Vector2i> HudShader::clicked_at(const double x, const double y,
                                             const int width, const int height) {
        // Convert to Open GL coordinates (-1 to 1) and inverted y
        double glx = (x / (double)width) * 2.0 - 1.0;
        double gly = (((double)height - y) / (double)height) * 2.0 - 1.0;

        double scale = 4.0/(double)columns;
        double xscale = (double)height / (double)width;

        // Convert to inventory positions
        double ix = (glx + 2.0 * xscale * screen_area) / (scale * xscale * screen_area);
        double iy = (gly + 1.0) / (scale * screen_area);

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
                           const BlockTypeInfo &blocks) {
        bind([&](Context c) {
                float scale = 4.0f/(float)columns;
                float xscale = (float)height / (float)width;

                c.blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                /* Set up for 17 x 14 HUD grid */
                c.set(matrix, hud_translation_matrix(scale, xscale, screen_area));
                c.set(offset, hud_offset_vector(xscale, screen_area));

                /* Use background texture*/
                c.set(sampler, texture);

                /* Generate and draw background model */

                HudModel hm(hud.backgrounds(), position, normal, uv);
                c.enable(GL_BLEND);
                c.draw(hm);
                c.disable(GL_BLEND);

                c.enable(GL_DEPTH_TEST);
                c.enable(GL_CULL_FACE);

                /* Use block texture */
                c.set(sampler, block_texture);
                /* Generate and draw item stacks */
                ItemStackModel ism(position, normal, uv, hud.stacks(), blocks);
                c.draw(ism);

                /* Use font texture */
                c.set(sampler, font_texture);


                /* Check for held block*/
                auto held = hud.held();
                if(held && hud.get_interactive()) {

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
                    BlockModel bm(position, normal, uv, held->type, x / xscale, y,
                                  scale * xscale * screen_area * 0.55, blocks);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    c.draw(bm);
                }
                c.disable(GL_CULL_FACE);
                c.disable(GL_DEPTH_TEST);

                /* Set up for 17 x 14 HUD grid */
                c.set(matrix,  hud_translation_matrix(scale, xscale, screen_area));
                c.set(offset, hud_offset_vector(xscale, screen_area));

                /* Use font texture */
                c.set(sampler, font_texture);

                /* Generate and draw item stack amounts */
                AmountModel am(position, normal, uv, hud.stacks());
                c.enable(GL_BLEND);
                c.draw(am);
                c.disable(GL_BLEND);
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
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);

                m.push_back(1.0f+i);  m.push_back(1.0f+j);  m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);

                m.push_back(-0.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);

                m.push_back(-0.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);

                m.push_back(1.0f+i);  m.push_back(1.0f+j);  m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(1.0f);
                m.push_back(0.0f); m.push_back(0.0f);

                m.push_back(1.0f+i); m.push_back(-0.0f+j);  m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(1.0f); m.push_back(0.0f);
                m.push_back(t*ts + ts); m.push_back(0.0f);
                m.push_back(0.0f); m.push_back(0.0f);
            }
        }
        return m;
    }

    void make_block(int type, float x, float y, float z, float size,
                    float rx, float ry, float rz, float *d,
                    const BlockTypeInfo &blocks) {
        char ao[6][4] = {0};
        if(blocks.is_plant[type]) {
            make_rotated_cube(d, ao,
                              0, 0, 0, 0, 0, 1,
                              x, y, z, size, 0, 0, 0,
                              type, blocks.blocks);
        } else {
            make_rotated_cube(d, ao,
                              1, 1, 1, 1, 1, 1,
                              x, y, z, size, rx, ry, rz,
                              type, blocks.blocks);
        }
    }

    void make_stacks(const std::unordered_map<Vector2i, ItemStack, matrix_hash<Vector2i>> &stacks,
                     float *d,
                     const float rx,
                     const float ry,
                     const float rz,
                     const BlockTypeInfo &blocks) {

        int offset = 0;
        for (const auto &pair: stacks) {
            make_block(pair.second.type, pair.first[0] + 0.5, pair.first[1] + 0.45, 0, 0.3,
                       rx, ry, rz, d + offset, blocks);
            offset += blocks.is_plant[pair.second.type] ? 10 * 6 : 10 * 6 * 6;
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
                make_character(d + i * 10 * 6, pair.first[0] - (float)offset*0.2 + 0.75f, pair.first[1] + 0.25f, 0.1, 0.2, text[index], 0.0);
                i++;
            }
        }
    }

};
