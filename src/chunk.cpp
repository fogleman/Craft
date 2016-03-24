#include <iostream>
#include <memory>
#include <string.h>
#include "block.h"
#include "compress.h"
#include "chunk.h"
#include "matrix.h"

namespace konstructs {
    using nonstd::nullopt;

    int chunked_int(int p) {
        if(p < 0) {
            return (p - CHUNK_SIZE + 1) / CHUNK_SIZE;
        } else {
            return p / CHUNK_SIZE;
        }
    }

    int chunked(float p) {
        return chunked_int(roundf(p));
    }

    Vector3i chunked_vec_int(const Vector3i position) {
        return Vector3i(chunked_int(position[0]), chunked_int(position[2]), chunked_int(position[1]));
    }

    Vector3i chunked_vec(const Vector3f position) {
        return chunked_vec_int(position.cast<int>());
    }

    ChunkData::ChunkData(const Vector3i _position, char *compressed, const int size):
        position(_position) {
        blocks = new char[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        int out_size = inflate_data(compressed + BLOCKS_HEADER_SIZE,
                                    size - BLOCKS_HEADER_SIZE,
                                    blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
    }
    ChunkData::ChunkData() {
        blocks = new char[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        memset(blocks, (char)SOLID_BLOCK, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
    }
    ChunkData::ChunkData(const Vector3i position, char *blocks) :
        position(position), blocks(blocks) {}

    ChunkData::~ChunkData() {
        delete[] blocks;
    }

    char ChunkData::get(const Vector3i &pos) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        // TODO: Looking for a block in the wrong chunk is a bit weird, but hit code does it
        if(lx < CHUNK_SIZE && ly < CHUNK_SIZE && lz < CHUNK_SIZE &&
           lx >= 0 && ly >= 0 && lz >= 0) {
            return blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE];
        } else {
            return 0;
        }
    }

    std::shared_ptr<ChunkData> ChunkData::set(const Vector3i &pos, const char type) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        char *new_blocks = new char[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
        memcpy(new_blocks, blocks, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

        new_blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE] = type;

        return std::make_shared<ChunkData>(position, new_blocks);
    }

    /**
     * Using a camera position and a camera direction, find the
     * closest within max_distance that intersect the directional
     * vector.
     */
    optional<pair<Block, Block>> ChunkData::get(const Vector3f &camera_position,
                                                const Vector3f &camera_direction,
                                                const float max_distance,
                                                const BlockData &blocks) const {
        int m = 4;
        Vector3f pos = camera_position;
        Vector3i blockPos(0,0,0);
        for (int i = 0; i < max_distance * m; i++) {
            const Vector3i nBlockPos(roundf(pos[0]), roundf(pos[1]), roundf(pos[2]));
            if (nBlockPos != blockPos) {
                char hw = get(nBlockPos);
                if (blocks.is_obstacle[hw] || blocks.is_plant[hw]) {
                    return optional<pair<Block, Block>>(pair<Block, Block>(Block(blockPos, hw),
                                                                           Block(nBlockPos, hw)));
                }
                blockPos = nBlockPos;
            }
            pos += (camera_direction / m);
        }
        return nullopt;
    }
};
