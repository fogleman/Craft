#ifndef _konstructs_h_
#define _konstructs_h_

#define MAX_CHUNKS 8192 * 4
#define MAX_PENDING_CHUNKS 128
#define MAX_PLAYERS 128
#define WORKERS 4
#define MAX_TEXT_LENGTH 256
#define MAX_NAME_LENGTH 32
#define MAX_PATH_LENGTH 256
#define MAX_ADDR_LENGTH 256

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define WORKER_IDLE 0
#define WORKER_BUSY 1
#define WORKER_DONE 2

#include "tinycthread/tinycthread.h"
#include "util.h"
#include "matrix.h"
#include "item.h"
#include "chunk.h"
#include <sys/time.h>

typedef struct {
  int p;
  int q;
  int k;
  char neighbour_blocks[3][3][3][(CHUNK_SIZE + 2)*(CHUNK_SIZE + 2)*(CHUNK_SIZE + 2)];
  int faces;
  GLfloat *data;
} WorkerItem;

typedef struct {
  int index;
  int state;
  thrd_t thrd;
  mtx_t mtx;
  cnd_t cnd;
  WorkerItem item;
} Worker;

typedef struct {
  int x;
  int y;
  int z;
  int w;
} Block;

typedef struct {
  float x;
  float y;
  float z;
  float rx;
  float ry;
  float t;
} State;

typedef struct {
  int id;
  char name[MAX_NAME_LENGTH];
  State state;
  State state1;
  State state2;
  GLuint buffer;
} Player;

typedef struct {
  GLuint program;
  GLuint position;
  GLuint normal;
  GLuint uv;
  GLuint matrix;
  GLuint sampler;
  GLuint camera;
  GLuint timer;
  GLuint extra1;
  GLuint extra2;
  GLuint extra3;
  GLuint extra4;
  GLuint extra5;
} Attrib;

typedef struct {
  GLFWwindow *window;
  Worker workers[WORKERS];
  Chunk chunks[MAX_CHUNKS];
  int chunk_count;
  int create_radius;
  int render_radius;
  int delete_radius;
  Player players[MAX_PLAYERS];
  int player_count;
  int typing;
  char typing_buffer[MAX_TEXT_LENGTH];
  int message_index;
  char messages[MAX_MESSAGES][MAX_TEXT_LENGTH];
  int width;
  int height;
  int observe1;
  int observe2;
  int flying;
  int debug_screen;
  int inventory_screen;
  int scale;
  int ortho;
  float fov;
  int suppress_char;
  char server_addr[MAX_ADDR_LENGTH];
  int server_port;
  char server_user[32];
  char server_pass[64];
  int day_length;
  int time_changed;
  int blocks_recv;
  char *chunk_buffer;
  int chunk_buffer_size;
  int pending_chunks;
  FPS fps;
  char text_message[KONSTRUCTS_TEXT_MESSAGE_SIZE];
  char text_prompt[KONSTRUCTS_TEXT_MESSAGE_SIZE];
  int mouse_item;
} Model;

// in main.c
void draw_triangles_2d(Attrib *attrib, GLuint buffer, int count);
GLuint gen_plant_buffer(float x, float y, float z, float n, int w);
GLuint gen_cube_buffer(float x, float y, float z, float n, int w);
void draw_plant(Attrib *attrib, GLuint buffer);
void draw_cube(Attrib *attrib, GLuint buffer);
void print(Attrib *attrib, int justify, float x, float y, float n, char *text);

#endif
