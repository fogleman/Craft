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
        ChunkShader(const float fov, const GLuint block_texture, const GLuint damage_texture,
                    const GLuint sky_texture, const float near_distance, const string &vert_str,
                    const string &frag_str, const int max_radius);
        int size() const;
        void add(const shared_ptr<ChunkModelResult> &data);
        int render(const Player &p, const int width, const int height,
                   const float current_daylight, const float current_timer,
                   const int radius, const float view_distance, const Vector3i &player_chunk);
        const GLuint data_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint sampler;
        const GLuint sky_sampler;
        const GLuint damage_sampler;
        const GLuint fog_distance;
        const GLuint light_color;
        const GLuint ambient_light;
        const GLuint timer;
        const GLuint camera;
        const GLuint block_texture;
        const GLuint sky_texture;
        const GLuint damage_texture;
        const float near_distance;
    private:
        std::unordered_map<Vector3i, ChunkModel *, matrix_hash<Vector3i>> models;
        const float fov;
        const int max_radius;
    };

    bool chunk_visible(const float planes[6][4], const Vector3i &position);
};

#endif
