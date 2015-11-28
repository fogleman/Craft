#ifndef __SKY_SHADER_H__
#define __SKY_SHADER_H__

#include "player.h"
#include "shader.h"
#include "matrix.h"

namespace konstructs {

    class SkyModel : public Attribute {
    public:
        SkyModel(GLuint _position_attr, GLuint _uv_attr);
        virtual void bind();
        const GLuint vertices;
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
        GLuint buffer;
    };

    class SkyShader : public ShaderProgram {
    public:
        SkyShader(const int _radius, const float _fov, const GLuint _sky_texture);
        void render(const Player &p, const int width, const int height,
                    const float current_timer);
        const GLuint position_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint sampler;
        const GLuint timer;
        const GLuint sky_texture;
    private:
        const int radius;
        const float fov;
        SkyModel model;
    };

};

#endif
