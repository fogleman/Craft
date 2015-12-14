#ifndef __HUDSHADER_H__
#define __HUDSHADER_H__
#include <memory>
#include "shader.h"
namespace konstructs {

    class HudModel : public BufferModel {
    public:
        HudModel(const int *background,
                 GLuint _position_attr, GLuint color_attr,
                 const int columns, const int rows);
        virtual void bind();
    private:
        const GLuint position_attr;
        const GLuint color_attr;
    };

    class HudShader: private ShaderProgram {
    public:
        HudShader(int columns, int rows);
        void render(const int width, const int height);

    private:
        const GLuint position;
        const GLuint scale;
        const GLuint xscale;
        const GLuint color;
        const int columns;
        const int rows;
    };
};
#endif
