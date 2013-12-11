#ifndef _main_h_
#define _main_h_

#import "map.h"

typedef struct {
    Map map;
    int p;
    int q;
    int faces;
    int dirty;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint uv_buffer;
} Chunk;

typedef struct {
    int id;
    float x;
    float y;
    float z;
    float rx;
    float ry;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint uv_buffer;
} Player;

int get_block(Chunk *chunks, int chunk_count, int x, int y, int z);

#endif