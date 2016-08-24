#include <iostream>
#include <memory>
#include <string.h>
#include "block.h"
#include "compress.h"
#include "chunk.h"
#include "matrix.h"

namespace konstructs {
    using nonstd::nullopt;

    std::shared_ptr<ChunkData> SOLID_CHUNK(std::make_shared<ChunkData>(SOLID_TYPE));
    std::shared_ptr<ChunkData> VACUUM_CHUNK(std::make_shared<ChunkData>(VACUUM_TYPE));

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

    ChunkData::ChunkData(const Vector3i position, char *compressed, const int size, uint8_t *buffer):
        position(position) {
        blocks = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        int out_size = inflate_data(compressed + BLOCKS_HEADER_SIZE,
                                    size - BLOCKS_HEADER_SIZE,
                                    (char*)buffer, BLOCK_BUFFER_SIZE);
        revision =
            buffer[2] +
            (buffer[2 + 1] << 8) +
            (buffer[2 + 2] << 16) +
            (buffer[2 + 3] << 24);

        for(int i = 0; i < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; i++) {
            blocks[i].type = buffer[i * BLOCK_SIZE] + (buffer[i * BLOCK_SIZE + 1] << 8);
            blocks[i].health = buffer[i * BLOCK_SIZE + 2] + ((buffer[i * BLOCK_SIZE + 3] & 0x07) << 8);
            blocks[i].direction = (buffer[i * BLOCK_SIZE + 3] & 0xE0) >> 5;
            blocks[i].rotation = (buffer[i * BLOCK_SIZE + 3] & 0x18) >> 3;
        }
    }

    ChunkData::ChunkData(const uint16_t type) : revision(0) {
        blocks = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        for(int i = 0; i < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; i++) {
            blocks[i].type = type;
            blocks[i].health = MAX_HEALTH;
            blocks[i].direction = DIRECTION_UP;
            blocks[i].rotation = ROTATION_IDENTITY;
        }
    }

    ChunkData::ChunkData(const Vector3i position, const uint32_t revision, BlockData *blocks) :
        position(position), revision(revision), blocks(blocks) {}

    ChunkData::~ChunkData() {
        delete[] blocks;
    }

    BlockData ChunkData::get(const Vector3i &pos) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        // TODO: Looking for a block in the wrong chunk is a bit weird, but hit code does it
        if(lx < CHUNK_SIZE && ly < CHUNK_SIZE && lz < CHUNK_SIZE &&
           lx >= 0 && ly >= 0 && lz >= 0) {
            int i = lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE;
            return blocks[i];
        } else {
            return {0, 0};
        }
    }

    std::shared_ptr<ChunkData> ChunkData::set(const Vector3i &pos, const BlockData &data) const {
        int lx = pos[0] - position[0] * CHUNK_SIZE;
        int ly = pos[1] - position[2] * CHUNK_SIZE;
        int lz = pos[2] - position[1] * CHUNK_SIZE;

        BlockData *new_blocks = new BlockData[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
        memcpy(new_blocks, blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*sizeof(BlockData));

        new_blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE] = data;

        // A chunk that we altered ourselves is treated as invalid

        return std::make_shared<ChunkData>(position, 0, new_blocks);
    }

    /**
     * Using a camera position and a camera direction, find the
     * closest within max_distance that intersect the directional
     * vector.
     */
    optional<pair<Block, Block>> ChunkData::get(const Vector3f &camera_position,
                                                const Vector3f &camera_direction,
                                                const float max_distance,
                                                const BlockTypeInfo &blocks) const {
        int m = 4;
        Vector3f pos = camera_position;
        Vector3i blockPos(0,0,0);
        for (int i = 0; i < max_distance * m; i++) {
            const Vector3i nBlockPos(roundf(pos[0]), roundf(pos[1]), roundf(pos[2]));
            if (nBlockPos != blockPos) {
                BlockData data = get(nBlockPos);
                if (blocks.is_obstacle[data.type] || blocks.is_plant[data.type]) {
                    return optional<pair<Block, Block>>(pair<Block, Block>(Block(blockPos, data),
                                                                           Block(nBlockPos, data)));
                }
                blockPos = nBlockPos;
            }
            pos += (camera_direction / m);
        }
        return nullopt;
    }
};
