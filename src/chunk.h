#ifndef _chunk_h_
#define _chunk_h_

#include <utility>
#include <Eigen/Geometry>
#include "optional.hpp"
#include "shader.h" //TODO: remove
#include "block.h"

#define CHUNK_SIZE 32
#define BLOCKS_HEADER_SIZE 2

namespace konstructs {
    using namespace Eigen;
    using nonstd::optional;
    using std::pair;

    int chunked_int(int p);

    int chunked(float p);

    Vector3i chunked_vec_int(const Vector3i position);

    Vector3i chunked_vec(const Vector3f position);

    class ChunkData {
    public:
        ChunkData(const Vector3i _position, char *compressed, const int size);
        ChunkData(const Vector3i position, char *blocks);
        ChunkData();
        ~ChunkData();
        char get(const Vector3i &pos) const;
        std::shared_ptr<ChunkData> set(const Vector3i &pos, const char type) const;
        optional<pair<Block, Block>> get(const Vector3f &camera_position,
                                         const Vector3f &camera_direction,
                                         const float max_distance,
                                         const BlockData &blocks) const;
        const Vector3i position;
        char *blocks;
    };
};

#define CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) \
  for(int ex = 0; ex < CHUNK_SIZE; ex++) { \
    for(int ey = 0; ey < CHUNK_SIZE; ey++) { \
      for(int ez = 0; ez < CHUNK_SIZE; ez++) { \
        int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define END_CHUNK_FOR_EACH \
       } \
     } \
   }

#define CHUNK_FOR_EACH_YZ(blocks, x, ex, ey, ez, ew)     \
  for(int ey = 0; ey < CHUNK_SIZE; ey++) { \
    for(int ez = 0; ez < CHUNK_SIZE; ez++) { \
      int ex = x; \
      int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define CHUNK_FOR_EACH_XY(blocks, z, ex, ey, ez, ew)     \
  for(int ex = 0; ex < CHUNK_SIZE; ex++) { \
    for(int ey = 0; ey < CHUNK_SIZE; ey++) { \
      int ez = z; \
      int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define CHUNK_FOR_EACH_XZ(blocks, y, ex, ey, ez, ew)     \
  for(int ex = 0; ex < CHUNK_SIZE; ex++) { \
    for(int ez = 0; ez < CHUNK_SIZE; ez++) { \
      int ey = y; \
      int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define END_CHUNK_FOR_EACH_2D \
     } \
   }

#define CHUNK_FOR_EACH_X(blocks, y, z, ex, ey, ez, ew)   \
  for(int ex = 0; ex < CHUNK_SIZE; ex++) { \
    int ey = y; \
    int ez = z; \
    int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define CHUNK_FOR_EACH_Y(blocks, x, z, ex, ey, ez, ew)   \
  for(int ey = 0; ey < CHUNK_SIZE; ey++) { \
    int ez = z; \
    int ex = x; \
    int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define CHUNK_FOR_EACH_Z(blocks, x, y, ex, ey, ez, ew)   \
  for(int ez = 0; ez < CHUNK_SIZE; ez++) { \
    int ex = x; \
    int ey = y; \
    int ew = blocks[ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];

#define END_CHUNK_FOR_EACH_1D \
  }

#endif
