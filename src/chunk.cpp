#include <iostream>
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

    /**
     * Using a camera position and a camera direction, find the
     * closest within max_distance that intersect the directional
     * vector.
     */
    optional<Block> ChunkData::get(const Vector3f &camera_position,
                                   const Vector3f &camera_direction,
                                   const float max_distance, const bool previous,
                                   const BlockData &blocks) const {
        int m = 32;
        Vector3f pos = camera_position;
        Vector3i blockPos(0,0,0);
        for (int i = 0; i < max_distance * m; i++) {
            const Vector3i nBlockPos(roundf(pos[0]), roundf(pos[1]), roundf(pos[2]));
            if (nBlockPos != blockPos) {
                char hw = get(nBlockPos);
                if (blocks.is_obstacle[hw] || blocks.is_plant[hw]) {
                    if (previous) {
                        return optional<Block>(Block(blockPos, hw));
                    } else {
                        return optional<Block>(Block(nBlockPos, hw));
                    }
                }
                blockPos = nBlockPos;
            }
            pos += (camera_direction / m);
        }
        return nullopt;
    }
};
