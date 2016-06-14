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

    class ChunkModel : public BufferModel {
    public:
        ChunkModel(const shared_ptr<ChunkModelResult> &data,
                   GLuint data_attr);
        virtual void bind();
        virtual int vertices();
        const Vector3i position;
        const int faces;
        Matrix4f translation;
    private:
        const GLuint data_attr;
    };

    class ChunkShader : public ShaderProgram {
    public:
        ChunkShader(const float _fov, const GLuint _block_texture,
                    const GLuint _sky_texture, const float _near_distance, const string &vert_str,
                    const string &frag_str);
        void add(const shared_ptr<ChunkModelResult> &data);
        int render(const Player &p, const int width, const int height,
                   const float current_daylight, const float current_timer,
                   World &world, Client &client, const int radius);
        void delete_unused_models(const Vector3f position, const int radi);
        const GLuint data_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint sampler;
        const GLuint sky_sampler;
        const GLuint fog_distance;
        const GLuint light_color;
        const GLuint ambient_light;
        const GLuint timer;
        const GLuint camera;
        const GLuint block_texture;
        const GLuint sky_texture;
        const float near_distance;
    private:
        std::unordered_map<Vector3i, ChunkModel *, matrix_hash<Vector3i>> models;
        const float fov;
    };

    bool chunk_visible(const float planes[6][4], const Vector3i &position);
};

#endif
