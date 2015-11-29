#include "world.h"

namespace konstructs {
    void World::insert(const Vector3i &pos, std::shared_ptr<ChunkData> data) {
        chunks.insert({pos, data});
    }
    const std::shared_ptr<ChunkData> World::at(const Vector3i &pos) const {
        return chunks.at(pos);
    }
    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::find(const Vector3i &pos) const {
        return chunks.find(pos);
    }
    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::end() const {
        return chunks.end();
    }
};
