#ifndef __CHUNK_SHADER_H__
#define __CHUNK_SHADER_H__

#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "shader.h"
#include "player.h"
#include "chunk.h"
#define WORKERS 4

namespace konstructs {

    class ChunkModel : public Attribute {
    public:
        ChunkModel(Vector3f _position, float *data, size_t size,
                   GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr);
        Matrix4f translation();
        virtual void bind();
        const size_t size;
    private:
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        GLuint buffer;
        Vector3f position;
    };

    class ChunkShader : public ShaderProgram {
    public:
        ChunkShader();
        void add(Vector3f position, float *data, size_t size);
        void render(const Player &p, int width, int height);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint view;
    private:
        std::vector<ChunkModel *> models;
    };

    /*
        const GLuint camera;
        const GLuint fog_distance;
        const GLuint translation;
        const GLuint timer;
        const GLuint daylight;
*/

    class ChunkModelFactory {
    public:
        ChunkModelFactory();
        void create_model(std::shared_ptr<ChunkData> data);
        std::vector<std::shared_ptr<ChunkModel>> fetch_models();
    private:
        void worker();
        std::mutex mutex;
        std::condition_variable chunks_condition;
        std::queue<std::shared_ptr<ChunkData>> chunks;
        std::vector<std::shared_ptr<ChunkModel>> models;
    };
};

#endif
