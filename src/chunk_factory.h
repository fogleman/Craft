#ifndef __CHUNK_FACTORY_H__
#define __CHUNK_FACTORY_H__

#include <memory>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include "chunk.h"
#include "world.h"
#include "matrix.h"

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
                         const int _faces);
        ~ChunkModelResult();
        const Vector3i position;
        const int size;
        const int faces;
        GLfloat *data();
    private:
        GLfloat *mData;
    };

    class ChunkModelFactory {
    public:
        ChunkModelFactory(const BlockData &_block_data);
        void create_models(const std::vector<Vector3i> &positions,
                           const World &world);
        std::vector<std::shared_ptr<ChunkModelResult>> fetch_models();
    private:
        void worker();
        std::mutex mutex;
        std::condition_variable chunks_condition;
        std::queue<ChunkModelData> chunks;
        std::vector<std::shared_ptr<ChunkModelResult>> models;
        const BlockData &block_data;
    };
    const std::shared_ptr<ChunkData> get_chunk(const Vector3i &position,
                                               const World &world);
    const ChunkModelData create_model_data(const Vector3i &position,
                                           const World &world);

    shared_ptr<ChunkModelResult> compute_chunk(const ChunkModelData &data, const BlockData &block_data);
};
#endif
