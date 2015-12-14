#ifndef __CROSSHAIR_SHADER_H__
#define __CROSSHAIR_SHADER_H__
#include "shader.h"

namespace konstructs {

    class CrosshairShader : private ShaderProgram {
    public:
        CrosshairShader();
        void render(const int width, const int height);
    private:
        const GLuint projection;
        const GLuint position;
        const GLuint color;
    };
};
#endif
