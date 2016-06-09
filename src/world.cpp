#include "world.h"

namespace konstructs {
    void World::request_chunk(const Vector3i &pos, Client &client) {
        /* Keep track of the chunk so we don't request it again */
        requested.insert(pos);
        /* Remove requested chunks from updated (we will get the latest update) */
        updated.erase(pos);
        client.chunk(pos);
    }

    void World::set_chunk_updated(const Vector3i &pos) {
        updated.insert(pos);
    }

    bool World::chunk_not_requested(const Vector3i &pos) const {
        return (updated.find(pos) != updated.end() && chunks.find(pos) != chunks.end()) // Updated and exist in the world
            || (requested.find(pos) == requested.end() && chunks.find(pos) == chunks.end()); // Not requested and not in world
    }

    bool World::chunk_updated_since_requested(const Vector3i &pos) const {
        return updated.find(pos) != updated.end();
    }

    void World::delete_unused_chunks(const Vector3f position, const int radi) {
        for ( auto it = chunks.begin(); it != chunks.end();) {
            if ((it->second->position - chunked_vec(position)).norm() > radi) {
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

        /* We now have the chunk, so we don't need to keep track of
         * it being requested.
         */
        requested.erase(pos);
    }

    const char World::get_block(const Vector3i &block_pos) const {
        return chunks.at(chunked_vec_int(block_pos))->get(block_pos);
    }

    const std::shared_ptr<ChunkData> World::chunk_at(const Vector3i &block_pos) const {
        return chunks.at(chunked_vec_int(block_pos));
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
