#ifndef __CHUNK_SHADER_H__
#define __CHUNK_SHADER_H__

#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>

#include "shader.h"
#include "player.h"
#include "chunk.h"
#include "matrix.h"

#define WORKERS 4

namespace konstructs {
    using std::shared_ptr;

    struct ChunkModelData {
        const Vector3i position;
        const shared_ptr<ChunkData> below;
        const shared_ptr<ChunkData> above;
        const shared_ptr<ChunkData> left;
        const shared_ptr<ChunkData> right;
        const shared_ptr<ChunkData> front;
        const shared_ptr<ChunkData> back;
        const shared_ptr<ChunkData> above_left;
        const shared_ptr<ChunkData> above_right;
        const shared_ptr<ChunkData> above_front;
        const shared_ptr<ChunkData> above_back;
        const shared_ptr<ChunkData> above_left_front;
        const shared_ptr<ChunkData> above_right_front;
        const shared_ptr<ChunkData> above_left_back;
        const shared_ptr<ChunkData> above_right_back;
        const shared_ptr<ChunkData> left_front;
        const shared_ptr<ChunkData> left_back;
        const shared_ptr<ChunkData> right_front;
        const shared_ptr<ChunkData> right_back;
        const shared_ptr<ChunkData> self;
    };

    class ChunkModelResult {
    public:
        ChunkModelResult(const Vector3i _position, const int components,
                         const int faces);
        ~ChunkModelResult();
        const Vector3i position;
        const int size;
        GLfloat *data();
    private:
        GLfloat *mData;
    };

    class ChunkModel : public Attribute {
    public:
        ChunkModel(const shared_ptr<ChunkModelResult> data,
                   GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr);
        Matrix4f translation();
        virtual void bind();
        Vector3i pos();
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
        std::vector<ChunkModel *> models;
    };

    class ChunkModelFactory {
    public:
        ChunkModelFactory(const BlockData &_block_data);
        void create_model(const Vector3i &position,
                          const std::unordered_map<Vector3i, shared_ptr<ChunkData>, matrix_hash<Vector3i>> &chunks);
        std::vector<std::shared_ptr<ChunkModelResult>> fetch_models();
    private:
        void worker();
        const std::shared_ptr<ChunkData> get_chunk(const Vector3i &position,
                                                   const std::unordered_map<Vector3i, shared_ptr<ChunkData>, matrix_hash<Vector3i>> &chunks);
        std::mutex mutex;
        std::condition_variable chunks_condition;
        std::queue<ChunkModelData> chunks;
        std::vector<std::shared_ptr<ChunkModelResult>> models;
        const Vector3i BELOW;
        const Vector3i ABOVE;
        const Vector3i LEFT;
        const Vector3i RIGHT;
        const Vector3i FRONT;
        const Vector3i BACK;
        const Vector3i ABOVE_LEFT;
        const Vector3i ABOVE_RIGHT;
        const Vector3i ABOVE_FRONT;
        const Vector3i ABOVE_BACK;
        const Vector3i ABOVE_LEFT_FRONT;
        const Vector3i ABOVE_RIGHT_FRONT;
        const Vector3i ABOVE_LEFT_BACK;
        const Vector3i ABOVE_RIGHT_BACK;
        const Vector3i LEFT_FRONT;
        const Vector3i RIGHT_FRONT;
        const Vector3i LEFT_BACK;
        const Vector3i RIGHT_BACK;
        const shared_ptr<ChunkData> EMPTY_CHUNK;
        const BlockData &block_data;
    };
    shared_ptr<ChunkModelResult> compute_chunk(const ChunkModelData &data, const BlockData &block_data);
    int chunk_visible(float planes[6][4], int p, int q, int k);
};

#endif
