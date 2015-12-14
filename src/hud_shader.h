#ifndef __HUDSHADER_H__
#define __HUDSHADER_H__
#include <vector>
#include "shader.h"

namespace konstructs {

    class HudModel : public BufferModel {
    public:
        HudModel(const std::vector<int> &background,
                 const GLuint position_attr, const GLuint uv_attr,
                 const int columns, const int rows);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        int verts;
    };

    class HudShader: private ShaderProgram {
    public:
        HudShader(const int columns, const int rows, const int texture);
        void render(const int width, const int height, const std::vector<int> &background);

    private:
        const GLuint position;
        const GLuint scale;
        const GLuint xscale;
        const GLuint sampler;
        const GLuint uv;
        const int texture;
        const int columns;
        const int rows;
    };
};
#endif
