#ifndef __WORLD_H__
#define __WORLD_H__

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "matrix.h"
#include "client.h"
#include "chunk.h"

namespace konstructs {
    class World {
    public:
        void request_chunk(const Vector3i &pos, Client &client);
        void set_chunk_updated(const Vector3i &pos);
        bool chunk_not_requested(const Vector3i &pos) const;
        bool chunk_updated_since_requested(const Vector3i &pos) const;
        void delete_unused_chunks(const Vector3f position, const int radi);
        void insert(std::shared_ptr<ChunkData> data);
        const char get_block(const Vector3i &block_pos) const;
        const std::shared_ptr<ChunkData> chunk_at(const Vector3i &block_pos) const;
        const std::shared_ptr<ChunkData> chunk(const Vector3i &chunk_pos) const;
        const std::vector<std::shared_ptr<ChunkData>> atAndAround(const Vector3i &pos) const;
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator find(const Vector3i &pos) const;
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator end() const;
    private:
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>> chunks;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> requested;
        std::unordered_set<Vector3i, matrix_hash<Vector3i>> updated;
    };
};

#endif
