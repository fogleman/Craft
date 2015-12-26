#ifndef __SELECTION_SHADER_H__
#define __SELECTION_SHADER_H__
#include <Eigen/Geometry>
#include "shader.h"
#include "player.h"
#include "matrix.h"

namespace konstructs {

    class SelectionShader : public ShaderProgram {
    public:
        SelectionShader(const int _radius, const float _fov,
                        const float _near_distance, const float scale);
        void render(const Player &p, const int width, const int height,
                    const Vector3i &selected);
        const GLuint position_attr;
        const GLuint matrix;
        const GLuint translation;
        const float near_distance;
    private:
        EigenModel model;
        const int radius;
        const float fov;
    };

    bool chunk_visible(const float planes[6][4], const Vector3i &position);
};

#endif
