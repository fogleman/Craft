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
        ChunkModel(const shared_ptr<ChunkModelResult> data,
                   GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr);
        Matrix4f translation();
        virtual void bind();
        const Vector3i pos() const;
        const size_t size;
    private:
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        GLuint buffer;
        Vector3i position;
    };

    class ChunkShader : public ShaderProgram {
    public:
        ChunkShader();
        void add(const shared_ptr<ChunkModelResult> data);
        int render(const Player &p, int width, int height);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint camera;
        const GLuint fog_distance;
        const GLuint sampler;
        const GLuint sky_sampler;
        const GLuint timer;
        const GLuint daylight;
    private:
        std::unordered_map<Vector3i, ChunkModel *, matrix_hash<Vector3i>> models;
    };

    int chunk_visible(float planes[6][4], int p, int q, int k);
};

#endif
