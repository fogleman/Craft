#include "world.h"

namespace konstructs {
    void World::insert(std::shared_ptr<ChunkData> data) {
        const Vector3i pos = data->position;
        chunks.erase(pos);
        chunks.insert({pos, data});
    }

    const std::shared_ptr<ChunkData> World::chunk_at(const Vector3i &block_pos) const {
        Vector3i chunk_pos(chunked_int(block_pos[0]),
                           chunked_int(block_pos[2]),
                           chunked_int(block_pos[1]));
        return chunks.at(chunk_pos);
    }

    const std::shared_ptr<ChunkData> World::chunk(const Vector3i &chunk_pos) const {
        return chunks.at(chunk_pos);
    }

    const std::vector<std::shared_ptr<ChunkData>> World::atAndAround(const Vector3i &pos) const {
        std::vector<std::shared_ptr<ChunkData>> result;
        for(int i = -1; i <= 1; i++) {
            for(int j = -1; j <= 1; j++) {
                for(int k = -1; k <= 1; k++) {
                    try {
                        result.push_back(chunks.at(pos + Vector3i(i, j, k)));
                    } catch(std::out_of_range e) {
                        // Not in this chunk
                    }
                }
            }
        }
        return result;
    }

    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::find(const Vector3i &pos) const {
        return chunks.find(pos);
    }

    std::unordered_map<Vector3i, std::shared_ptr<ChunkData>, matrix_hash<Vector3i>>::const_iterator World::end() const {
        return chunks.end();
    }

};
