#ifndef __CROSSHAIR_H__
#define __CROSSHAIR_H__
#include <memory>
#include "shader.h"
namespace konstructs {

    class Crosshair : private ShaderProgram {
    public:
        Crosshair();
        void render(const int width, const int height);
    private:
        const GLuint projection;
        const GLuint position;
        const GLuint color;
    };
};
#endif
