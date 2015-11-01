#ifndef __CROSSHAIR_H__
#define __CROSSHAIR_H__
#include <memory>
#include "shader.h"
namespace konstructs {

    class Crosshair : private ShaderProgram {
    public:
        Crosshair(const int _height, const int _width) :
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
            color(uniformId("color")),
            height(_height),
            width(_width) {
            float p = 0.03;
            MatrixXf segments(2, 4);
            segments.col(0) <<  0, -p;
            segments.col(1) <<  0,  p;
            segments.col(2) << -p,  0;
            segments.col(3) <<  p,  0;
            data = std::make_shared<EigenAttribute>(position, segments);
        }
        void render();

    private:
        const GLuint projection;
        const GLuint position;
        const GLuint color;
        int height;
        int width;
        std::shared_ptr<Attribute> data;
    };
};
#endif
