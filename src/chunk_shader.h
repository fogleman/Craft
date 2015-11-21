#ifndef __CHUNK_SHADER_H__
#define __CHUNK_SHADER_H__

#include <memory>
#include <unordered_map>
#include "shader.h"
#include "player.h"
#include "chunk.h"
#include "chunk_factory.h"
#include "matrix.h"

namespace konstructs {
    using std::shared_ptr;

    class ChunkModel : public Attribute {
    public:
        ChunkModel(const shared_ptr<ChunkModelResult> &data,
                   GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr);
        virtual void bind();
        const Vector3i position;
        const size_t size;
        const int faces;
        Matrix4f translation;
    private:
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        GLuint buffer;
    };

    class ChunkShader : public ShaderProgram {
    public:
        ChunkShader(const int _radius, const float _fov);
        void add(const shared_ptr<ChunkModelResult> &data);
        int render(const Player &p, int width, int height);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint sampler;
        const GLuint daylight;
    private:
        std::unordered_map<Vector3i, ChunkModel *, matrix_hash<Vector3i>> models;
        const int radius;
        const float fov;
    };

    bool chunk_visible(const float planes[6][4], const Vector3i &position);
};

#endif
