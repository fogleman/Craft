#include "matrix.h"
#include "crosshair_shader.h"

namespace konstructs {
    using matrix::projection_2d;
    CrosshairShader::CrosshairShader() :
        ShaderProgram(
            "crosshair",
            "#version 330\n"
            "uniform mat4 projection;\n"
            "in vec4 position;\n"
            "void main() {\n"
            "    gl_Position = projection * position;\n"
            "}\n",
            "#version 330\n"
            "uniform vec4 color;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    fragColor = vec4(color);\n"
            "}\n",
            GL_LINES),
        projection(uniformId("projection")),
        position(attributeId("position")),
        color(uniformId("color")) { }

    void CrosshairShader::render(const int width, const int height) {
        bind([&](Context c) {
                float p = 0.03;
                MatrixXf segments(2, 4);
                segments.col(0) <<  0, -p;
                segments.col(1) <<  0,  p;
                segments.col(2) << -p,  0;
                segments.col(3) <<  p,  0;
                EigenModel data(position, segments);
                c.logic_op(GL_INVERT);
                c.enable(GL_COLOR_LOGIC_OP);
                c.set(projection, projection_2d(width, height));
                c.set(color, Vector4f( 0.0, 0.0, 0.0, 1.0));
                c.draw(data);
                c.disable(GL_COLOR_LOGIC_OP);
            });
    }
};
