#include "world.h"

namespace konstructs {

    using nonstd::nullopt;

    int World::size() const {
        return chunks.size();
    }

    void World::delete_unused_chunks(const Vector3i player_chunk, const int radi) {
        for ( auto it = chunks.begin(); it != chunks.end();) {
            if ((it->second->position - player_chunk).norm() > radi) {
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void World::insert(std::shared_ptr<ChunkData> data) {
        /* Overwrite any existing chunk, we always want the latest data */
        const Vector3i pos = data->position;
        chunks.erase(pos);
        chunks.insert({pos, data});
    }

    const BlockData World::get_block(const Vector3i &block_pos) const {
        return chunk_at(block_pos)->get(block_pos);
    }

    const std::shared_ptr<ChunkData> World::chunk_at(const Vector3i &block_pos) const {
        try {
            return chunks.at(chunked_vec_int(block_pos));
        } catch(std::out_of_range e) {
            return VACUUM_CHUNK;
        }

    }

    const optional<std::shared_ptr<ChunkData>> World::chunk_opt(const Vector3i &chunk_pos) const {
        try {
            return chunks.at(chunk_pos);
        } catch(std::out_of_range e)  {
            return nullopt;
        }
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
