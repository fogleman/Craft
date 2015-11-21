#include <iostream>
#include "block.h"
#include "compress.h"
#include "chunk.h"
#include "matrix.h"
#include "config.h"

char chunk_get(Chunk *chunk, int x, int y, int z) {
  int lx = x - chunk->p * CHUNK_SIZE;
  int ly = y - chunk->k * CHUNK_SIZE;
  int lz = z - chunk->q * CHUNK_SIZE;

  // TODO: Looking for a block in the wrong chunk is a bit weird, but hit code does it
  if(lx < CHUNK_SIZE && ly < CHUNK_SIZE && lz < CHUNK_SIZE &&
     lx >= 0 && ly >= 0 && lz >= 0) {
    return chunk->blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE];
  } else {
    return 0;
  }
}

void chunk_set(Chunk *chunk, int x, int y, int z, int w) {
  int lx = x - chunk->p * CHUNK_SIZE;
  int ly = y - chunk->k * CHUNK_SIZE;
  int lz = z - chunk->q * CHUNK_SIZE;

  if(lx < CHUNK_SIZE && ly < CHUNK_SIZE && lz < CHUNK_SIZE &&
     lx >= 0 && ly >= 0 && lz >= 0) {
    chunk->blocks[lx+ly*CHUNK_SIZE+lz*CHUNK_SIZE*CHUNK_SIZE] = w;
  }
}

namespace konstructs {
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
};
