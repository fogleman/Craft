#include "matrix.h"
#include "hud_shader.h"

namespace konstructs {
    using matrix::projection_2d;
    using std::vector;

    vector<float> make_square(const int columns, const int rows,
                              const vector<int> &background);

    HudModel::HudModel(const vector<int>& background,
                       const GLuint position_attr, const GLuint uv_attr,
                       const int columns, const int rows) :
        position_attr(position_attr),
        uv_attr(uv_attr) {

        auto data = make_square(columns, rows, background);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat),
                     data.data(), GL_STATIC_DRAW);
        verts = data.size() / 4;
    }

    int HudModel::vertices() {
        return verts;
    }

    void HudModel::bind() {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glEnableVertexAttribArray(position_attr);
        glEnableVertexAttribArray(uv_attr);
        glVertexAttribPointer(position_attr, 2, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 4, 0);
        glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * 4, (GLvoid*)(sizeof(GLfloat)*2));
    }

    HudShader::HudShader(const int columns, const int rows, const int texture) :
        ShaderProgram(
            "hud",

            "#version 330\n"
            "uniform float scale;\n"
            "uniform float xscale;\n"
            "in vec2 position;\n"
            "in vec2 uv;\n"
            "out vec2 fragment_uv;\n"
            "void main() {\n"
            "    fragment_uv = uv;\n"
            "    vec4 pos = vec4(position.x*scale * 0.6 * xscale, position.y*scale*0.6 - 1.0, 0, 1);\n"
            "    gl_Position = vec4(pos.x - 2*xscale*0.6, pos.y, pos.z, pos.w);\n"
            "}\n",

            "#version 330\n"
            "uniform sampler2D sampler;\n"
            "in vec2 fragment_uv;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    fragColor = texture2D(sampler, fragment_uv);\n"
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

    void HudShader::render(const int width, const int height, const vector<int> &background) {
        bind([&](Context c) {
                HudModel m(background, position, uv, columns, rows);
                c.set(scale, 4.0f/(float)columns);
                c.set(xscale, (float)height / (float)width);
                c.set(sampler, texture);
                c.draw(m);
            });
    }

    vector<float> make_square(const int columns, const int rows,
                              const vector<int> &background) {
        vector<float> m;
        float ts = 0.25;
        for (int j=0; j<rows; j++) {
            for (int i=0; i<columns; i++) {
                int t = background[i+j*columns];
                if(t >= 0) {
                    m.push_back(-0.0f+i); m.push_back(1.0f+j);
                    m.push_back(t*ts); m.push_back(1.0f);
                    m.push_back(1.0f+i);  m.push_back(1.0f+j);
                    m.push_back(t*ts + ts); m.push_back(1.0f);
                    m.push_back(-0.0f+i); m.push_back(-0.0f+j);
                    m.push_back(t*ts); m.push_back(0.0f);
                    m.push_back(-0.0f+i); m.push_back(-0.0f+j);
                    m.push_back(t*ts); m.push_back(0.0f);
                    m.push_back(1.0f+i);  m.push_back(1.0f+j);
                    m.push_back(t*ts + ts); m.push_back(1.0f);
                    m.push_back(1.0f+i); m.push_back(-0.0f+j);
                    m.push_back(t*ts + ts); m.push_back(0.0f);
                }
            }
        }
        return m;
    }
};
