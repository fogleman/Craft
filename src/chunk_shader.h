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
        ~ChunkModel();
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
        ChunkShader(const int _radius, const float _fov, const GLuint _block_texture,
                    const GLuint _sky_texture, const float _near_distance);
        void add(const shared_ptr<ChunkModelResult> &data);
        int render(const Player &p, const int width, const int height,
                   const float current_daylight, const float current_timer);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint sampler;
        const GLuint sky_sampler;
        const GLuint fog_distance;
        const GLuint timer;
        const GLuint daylight;
        const GLuint camera;
        const GLuint block_texture;
        const GLuint sky_texture;
        const float near_distance;
    private:
        std::unordered_map<Vector3i, ChunkModel *, matrix_hash<Vector3i>> models;
        const int radius;
        const float fov;
    };

    bool chunk_visible(const float planes[6][4], const Vector3i &position);
};

#endif
