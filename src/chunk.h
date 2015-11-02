#ifndef _chunk_h_
#define _chunk_h_

#include <memory>

#include "shader.h"
#include "player.h"

#define CHUNK_SIZE 32

typedef struct {
  char blocks[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
  int p;
  int q;
  int k;
  int faces;
  int dirty;
  GLuint buffer;
} Chunk;

char chunk_get(Chunk *chunk, int x, int y, int z);
void chunk_set(Chunk *chunk, int x, int y, int z, int w);

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

namespace konstructs {

    class ChunkModel : public Attribute {
    public:
        ChunkModel(Vector3f _position, float *data, size_t size,
                   GLuint _position_attr, GLuint _normal_attr, GLuint _uv_attr);
        Matrix4f translation();
        virtual void bind();
        const size_t size;
    private:
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        GLuint buffer;
        Vector3f position;
    };

    class ChunkShader : public ShaderProgram {
    public:
        ChunkShader();
        void add(Vector3f position, float *data, size_t size);
        void render(const Player &p, int width, int height);
        const GLuint position_attr;
        const GLuint normal_attr;
        const GLuint uv_attr;
        const GLuint matrix;
        const GLuint translation;
        const GLuint view;
    private:
        std::vector<ChunkModel *> models;
    };

    /*
        const GLuint camera;
        const GLuint fog_distance;
        const GLuint translation;
        const GLuint timer;
        const GLuint daylight;
*/
};

#endif
