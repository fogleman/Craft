#ifndef __SKY_SHADER_H__
#define __SKY_SHADER_H__

#include "player.h"
#include "shader.h"
#include "matrix.h"

namespace konstructs {

    class SkyModel : public BufferModel {
    public:
        SkyModel(GLuint _position_attr, GLuint _uv_attr);
        virtual void bind();
        virtual int vertices();
    private:
        const GLuint position_attr;
        const GLuint uv_attr;
    };

    class SkyShader : public ShaderProgram {
    public:
        SkyShader(const float _fov, const GLuint _sky_texture,
                  const float _near_distance);
        void render(const Player &p, const int width, const int height,
                    const float current_timer, const int radius);
        const GLuint position_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint sampler;
        const GLuint timer;
        const GLuint sky_texture;
    private:
        const float fov;
        const float near_distance;
        SkyModel model;
    };

};

#endif
