#ifndef __WORLD_H__
#define __WORLD_H__

#include <unordered_map>
#include <memory>
#include "matrix.h"
#include "chunk.h"

namespace konstructs {
    class World {
    public:
        void insert(const Vector3i &pos, std::shared_ptr<ChunkData> data);
        const std::shared_ptr<ChunkData> at(const Vector3i &pos) const;
        const std::vector<std::shared_ptr<ChunkData>> atAndAround(const Vector3i &pos) const;
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator find(const Vector3i &pos) const;
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator end() const;
    private:
        std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>> chunks;
    };
};

#endif
