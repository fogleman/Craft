#ifndef _chunk_h_
#define _chunk_h_

#include <GLFW/glfw3.h>

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


#endif
