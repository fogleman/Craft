#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <curl/curl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "auth.h"
#include "chunk.h"
#include "client.h"
#include "config.h"
#include "cube.h"
#include "db.h"
#include "inventory.h"
#include "item.h"
#include "map.h"
#include "matrix.h"
#include "noise.h"
#include "parser.h"
#include "sign.h"
#include "tinycthread.h"
#include "util.h"
#include "worldgen/world.h"

#define MAX_CHUNKS 8192
#define MAX_PLAYERS 128
#define WORKERS 4
#define MAX_TEXT_LENGTH 256
#define MAX_NAME_LENGTH 32
#define MAX_PATH_LENGTH 256
#define MAX_ADDR_LENGTH 256

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define MODE_OFFLINE 0
#define MODE_ONLINE 1

#define WORKER_IDLE 0
#define WORKER_BUSY 1
#define WORKER_DONE 2

typedef struct {
    int p;
    int q;
    int load;
    Map *block_maps[3][3];
    Map *light_maps[3][3];
    int miny;
    int maxy;
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
    float r;
    float g;
    float b;
} SkyColor;

typedef struct {
    GLFWwindow *window;
    Worker workers[WORKERS];
    Chunk chunks[MAX_CHUNKS];
    int chunk_count;
    int create_radius;
    int render_radius;
    int delete_radius;
    int sign_radius;
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
    int item_index;
    int scale;
    int ortho;
    float fov;
    int suppress_char;
    int mode;
    int mode_changed;
    char db_path[MAX_PATH_LENGTH];
    char server_addr[MAX_ADDR_LENGTH];
    int server_port;
    int day_length;
    int time_changed;
    Block block0;
    Block block1;
    Block copy0;
    Block copy1;
    SkyColor sky_color;
    SkyColor last_sky_color;
    //int is_fullscreen;
    Inventory inventory;
} Model;

static Model model;
static Model *g = &model;

float time_of_day() {
    if (g->day_length <= 0) {
        return 0.5;
    }
    float t;
    t = glfwGetTime();
    t = t / g->day_length;
    t = t - (int)t;
    return t;
}

float get_daylight() {
    float timer = time_of_day();
    if (timer < 0.5) {
        float t = (timer - 0.25) * 100;
        return 1 / (1 + powf(2, -t));
    }
    else {
        float t = (timer - 0.85) * 100;
        return 1 - 1 / (1 + powf(2, -t));
    }
}

void get_sky_tint(Biome biome, SkyColor *c) {
    switch(biome) {
        //Not needed
        //case Biome_TEMPERATE:
        case Biome_DESERT:
            c->r = 1.4f;
            c->g = 1.2f,
            c->b = 1.0f;
            break;
        case Biome_RAINFOREST:
            c->r = 1.3f;
            c->g = 1.3f,
            c->b = 1.0f;
            break;
        case Biome_TAIGA:
            c->r = 0.7f;
            c->g = 0.8f,
            c->b = 0.9f;
            break;
        default:
            c->r = 1.0f;
            c->g = 1.0f,
            c->b = 1.0f;
            break;
    }
}

int get_scale_factor() {
    int window_width, window_height;
    int buffer_width, buffer_height;
    glfwGetWindowSize(g->window, &window_width, &window_height);
    glfwGetFramebufferSize(g->window, &buffer_width, &buffer_height);
    int result = buffer_width / window_width;
    result = MAX(1, result);
    result = MIN(2, result);
    return result;
}

void get_sight_vector(float rx, float ry, float *vx, float *vy, float *vz) {
    float m = cosf(ry);
    *vx = cosf(rx - RADIANS(90)) * m;
    *vy = sinf(ry);
    *vz = sinf(rx - RADIANS(90)) * m;
}

void get_motion_vector(int flying, int sz, int sx, float rx, float ry,
    float *vx, float *vy, float *vz) {
    *vx = 0; *vy = 0; *vz = 0;
    if (!sz && !sx) {
        return;
    }
    float strafe = atan2f(sz, sx);
    if (flying) {
        float m = cosf(ry);
        float y = sinf(ry);
        if (sx) {
            if (!sz) {
                y = 0;
            }
            m = 1;
        }
        if (sz > 0) {
            y = -y;
        }
        *vx = cosf(rx + strafe) * m;
        *vy = y;
        *vz = sinf(rx + strafe) * m;
    }
    else {
        *vx = cosf(rx + strafe);
        *vy = 0;
        *vz = sinf(rx + strafe);
    }
}

GLuint gen_crosshair_buffer() {
    int x = g->width / 2;
    int y = g->height / 2;
    int p = 10 * g->scale;
    float data[] = {
        x, y - p, x, y + p,
        x - p, y, x + p, y
    };
    return gen_buffer(sizeof(data), data);
}

GLuint gen_wireframe_buffer(float x, float y, float z, float n) {
    float data[72];
    make_cube_wireframe(data, x, y, z, n);
    return gen_buffer(sizeof(data), data);
}

GLuint gen_sky_buffer() {
    float data[12288];
    make_sphere(data, 1, 3);
    return gen_buffer(sizeof(data), data);
}

GLuint gen_cube_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(10, 6);
    float ao[6][4] = {0};
    float light[6][4] = {
        {0.5, 0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5}
    };
    make_cube(data, ao, light, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
    return gen_faces(10, 6, data);
}

GLuint gen_plant_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(10, 4);
    float ao = 0;
    float light = 1;
    make_plant(data, ao, light, x, y, z, n, w, 45);
    return gen_faces(10, 4, data);
}

GLuint gen_player_buffer(float x, float y, float z, float rx, float ry) {
    GLfloat *data = malloc_faces(10, 6);
    make_player(data, x, y, z, rx, ry);
    return gen_faces(10, 6, data);
}

GLuint gen_text_buffer(float x, float y, float n, char *text) {
    int length = strlen(text);
    GLfloat *data = malloc_faces(4, length);
    for (int i = 0; i < length; i++) {
        make_character(data + i * 24, x, y, n / 2, n, text[i]);
        x += n;
    }
    return gen_faces(4, length, data);
}

void draw_triangles_3d_ao(Attrib *attrib, GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 4, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 10, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_triangles_3d_text(Attrib *attrib, GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, 0);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 5, (GLvoid *)(sizeof(GLfloat) * 3));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_triangles_3d(Attrib *attrib, GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_triangles_2d(Attrib *attrib, GLuint buffer, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, 0);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 4, (GLvoid *)(sizeof(GLfloat) * 2));
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_lines(Attrib *attrib, GLuint buffer, int components, int count) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(attrib->position);
    glVertexAttribPointer(
        attrib->position, components, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_LINES, 0, count);
    glDisableVertexAttribArray(attrib->position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_chunk(Attrib *attrib, Chunk *chunk) {
    draw_triangles_3d_ao(attrib, chunk->buffer, chunk->faces * 6);
}

void draw_item(Attrib *attrib, GLuint buffer, int count) {
    draw_triangles_3d_ao(attrib, buffer, count);
}

void draw_text(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(attrib, buffer, length * 6);
    glDisable(GL_BLEND);
}

void draw_signs(Attrib *attrib, Chunk *chunk) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(attrib, chunk->sign_buffer, chunk->sign_faces * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void draw_sign(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-8, -1024);
    draw_triangles_3d_text(attrib, buffer, length * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void draw_cube(Attrib *attrib, GLuint buffer) {
    draw_item(attrib, buffer, 36);
}

void draw_plant(Attrib *attrib, GLuint buffer) {
    draw_item(attrib, buffer, 24);
}

void draw_player(Attrib *attrib, Player *player) {
    draw_cube(attrib, player->buffer);
}

Player *find_player(int id) {
    for (int i = 0; i < g->player_count; i++) {
        Player *player = g->players + i;
        if (player->id == id) {
            return player;
        }
    }
    return 0;
}

void update_player(Player *player,
    float x, float y, float z, float rx, float ry, int interpolate)
{
    if (interpolate) {
        State *s1 = &player->state1;
        State *s2 = &player->state2;
        memcpy(s1, s2, sizeof(State));
        s2->x = x; s2->y = y; s2->z = z; s2->rx = rx; s2->ry = ry;
        s2->t = glfwGetTime();
        if (s2->rx - s1->rx > PI) {
            s1->rx += 2 * PI;
        }
        if (s1->rx - s2->rx > PI) {
            s1->rx -= 2 * PI;
        }
    }
    else {
        State *s = &player->state;
        s->x = x; s->y = y; s->z = z; s->rx = rx; s->ry = ry;
        del_buffer(player->buffer);
        player->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
    }
}

void interpolate_player(Player *player) {
    State *s1 = &player->state1;
    State *s2 = &player->state2;
    float t1 = s2->t - s1->t;
    float t2 = glfwGetTime() - s2->t;
    t1 = MIN(t1, 1);
    t1 = MAX(t1, 0.1);
    float p = MIN(t2 / t1, 1);
    update_player(
        player,
        s1->x + (s2->x - s1->x) * p,
        s1->y + (s2->y - s1->y) * p,
        s1->z + (s2->z - s1->z) * p,
        s1->rx + (s2->rx - s1->rx) * p,
        s1->ry + (s2->ry - s1->ry) * p,
        0);
}

void delete_player(int id) {
    Player *player = find_player(id);
    if (!player) {
        return;
    }
    int count = g->player_count;
    del_buffer(player->buffer);
    Player *other = g->players + (--count);
    memcpy(player, other, sizeof(Player));
    g->player_count = count;
}

void delete_all_players() {
    for (int i = 0; i < g->player_count; i++) {
        Player *player = g->players + i;
        del_buffer(player->buffer);
    }
    g->player_count = 0;
}

float player_player_distance(Player *p1, Player *p2) {
    State *s1 = &p1->state;
    State *s2 = &p2->state;
    float x = s2->x - s1->x;
    float y = s2->y - s1->y;
    float z = s2->z - s1->z;
    return sqrtf(x * x + y * y + z * z);
}

float player_crosshair_distance(Player *p1, Player *p2) {
    State *s1 = &p1->state;
    State *s2 = &p2->state;
    float d = player_player_distance(p1, p2);
    float vx, vy, vz;
    get_sight_vector(s1->rx, s1->ry, &vx, &vy, &vz);
    vx *= d; vy *= d; vz *= d;
    float px, py, pz;
    px = s1->x + vx; py = s1->y + vy; pz = s1->z + vz;
    float x = s2->x - px;
    float y = s2->y - py;
    float z = s2->z - pz;
    return sqrtf(x * x + y * y + z * z);
}

Player *player_crosshair(Player *player) {
    Player *result = 0;
    float threshold = RADIANS(5);
    float best = 0;
    for (int i = 0; i < g->player_count; i++) {
        Player *other = g->players + i;
        if (other == player) {
            continue;
        }
        float p = player_crosshair_distance(player, other);
        float d = player_player_distance(player, other);
        if (d < 96 && p / d < threshold) {
            if (best == 0 || d < best) {
                best = d;
                result = other;
            }
        }
    }
    return result;
}

Chunk *find_chunk(int p, int q) {
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk->p == p && chunk->q == q) {
            return chunk;
        }
    }
    return 0;
}

int chunk_distance(Chunk *chunk, int p, int q) {
    int dp = ABS(chunk->p - p);
    int dq = ABS(chunk->q - q);
    return MAX(dp, dq);
}

int chunk_visible(float planes[6][4], int p, int q, int miny, int maxy) {
    int x = p * CHUNK_SIZE - 1;
    int z = q * CHUNK_SIZE - 1;
    int d = CHUNK_SIZE + 1;
    float points[8][3] = {
        {x + 0, miny, z + 0},
        {x + d, miny, z + 0},
        {x + 0, miny, z + d},
        {x + d, miny, z + d},
        {x + 0, maxy, z + 0},
        {x + d, maxy, z + 0},
        {x + 0, maxy, z + d},
        {x + d, maxy, z + d}
    };
    int n = g->ortho ? 4 : 6;
    for (int i = 0; i < n; i++) {
        int in = 0;
        int out = 0;
        for (int j = 0; j < 8; j++) {
            float d =
                planes[i][0] * points[j][0] +
                planes[i][1] * points[j][1] +
                planes[i][2] * points[j][2] +
                planes[i][3];
            if (d < 0) {
                out++;
            }
            else {
                in++;
            }
            if (in && out) {
                break;
            }
        }
        if (in == 0) {
            return 0;
        }
    }
    return 1;
}

int highest_block(float x, float z) {
    int result = -1;
    int nx = roundf(x);
    int nz = roundf(z);
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        MAP_FOR_EACH(map, ex, ey, ez, ew) {
            if (is_obstacle(ew) && ex == nx && ez == nz) {
                result = MAX(result, ey);
            }
        } END_MAP_FOR_EACH;
    }
    return result;
}

int _hit_test(
    Map *map, float max_distance, int previous,
    float x, float y, float z,
    float vx, float vy, float vz,
    int *hx, int *hy, int *hz)
{
    int m = 32;
    int px = 0;
    int py = 0;
    int pz = 0;
    for (int i = 0; i < max_distance * m; i++) {
        int nx = roundf(x);
        int ny = roundf(y);
        int nz = roundf(z);
        if (nx != px || ny != py || nz != pz) {
            int hw = map_get(map, nx, ny, nz);
            if (hw > 0) {
                if (previous) {
                    *hx = px; *hy = py; *hz = pz;
                }
                else {
                    *hx = nx; *hy = ny; *hz = nz;
                }
                return hw;
            }
            px = nx; py = ny; pz = nz;
        }
        x += vx / m; y += vy / m; z += vz / m;
    }
    return 0;
}

int hit_test(
    int previous, float x, float y, float z, float rx, float ry,
    int *bx, int *by, int *bz)
{
    int result = 0;
    float best = 0;
    int p = chunked(x);
    int q = chunked(z);
    float vx, vy, vz;
    get_sight_vector(rx, ry, &vx, &vy, &vz);
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk_distance(chunk, p, q) > 1) {
            continue;
        }
        int hx, hy, hz;
        int hw = _hit_test(&chunk->map, 8, previous,
            x, y, z, vx, vy, vz, &hx, &hy, &hz);
        if (hw > 0) {
            float d = sqrtf(
                powf(hx - x, 2) + powf(hy - y, 2) + powf(hz - z, 2));
            if (best == 0 || d < best) {
                best = d;
                *bx = hx; *by = hy; *bz = hz;
                result = hw;
            }
        }
    }
    return result;
}

int hit_test_face(Player *player, int *x, int *y, int *z, int *face) {
    State *s = &player->state;
    int w = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, x, y, z);
    if (is_obstacle(w)) {
        int hx, hy, hz;
        hit_test(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
        int dx = hx - *x;
        int dy = hy - *y;
        int dz = hz - *z;
        if (dx == -1 && dy == 0 && dz == 0) {
            *face = 0; return 1;
        }
        if (dx == 1 && dy == 0 && dz == 0) {
            *face = 1; return 1;
        }
        if (dx == 0 && dy == 0 && dz == -1) {
            *face = 2; return 1;
        }
        if (dx == 0 && dy == 0 && dz == 1) {
            *face = 3; return 1;
        }
        if (dx == 0 && dy == 1 && dz == 0) {
            int degrees = roundf(DEGREES(atan2f(s->x - hx, s->z - hz)));
            if (degrees < 0) {
                degrees += 360;
            }
            int top = ((degrees + 45) / 90) % 4;
            *face = 4 + top; return 1;
        }
    }
    return 0;
}

int collide(int height, float *x, float *y, float *z) {
    int result = 0;
    int p = chunked(*x);
    int q = chunked(*z);
    Chunk *chunk = find_chunk(p, q);
    if (!chunk) {
        return result;
    }
    Map *map = &chunk->map;
    int nx = roundf(*x);
    int ny = roundf(*y);
    int nz = roundf(*z);
    float px = *x - nx;
    float py = *y - ny;
    float pz = *z - nz;
    float pad = 0.25;
    for (int dy = 0; dy < height; dy++) {
        if (px < -pad && is_obstacle(map_get(map, nx - 1, ny - dy, nz))) {
            *x = nx - pad;
        }
        if (px > pad && is_obstacle(map_get(map, nx + 1, ny - dy, nz))) {
            *x = nx + pad;
        }
        if (py < -pad && is_obstacle(map_get(map, nx, ny - dy - 1, nz))) {
            *y = ny - pad;
            result = 1;
        }
        if (py > pad && is_obstacle(map_get(map, nx, ny - dy + 1, nz))) {
            *y = ny + pad;
            result = 1;
        }
        if (pz < -pad && is_obstacle(map_get(map, nx, ny - dy, nz - 1))) {
            *z = nz - pad;
        }
        if (pz > pad && is_obstacle(map_get(map, nx, ny - dy, nz + 1))) {
            *z = nz + pad;
        }
    }
    return result;
}

int player_intersects_block(
    int height,
    float x, float y, float z,
    int hx, int hy, int hz)
{
    int nx = roundf(x);
    int ny = roundf(y);
    int nz = roundf(z);
    for (int i = 0; i < height; i++) {
        if (nx == hx && ny - i == hy && nz == hz) {
            return 1;
        }
    }
    return 0;
}

int _gen_sign_buffer(
    GLfloat *data, float x, float y, float z, int face, const char *text)
{
    static const int glyph_dx[8] = {0, 0, -1, 1, 1, 0, -1, 0};
    static const int glyph_dz[8] = {1, -1, 0, 0, 0, -1, 0, 1};
    static const int line_dx[8] = {0, 0, 0, 0, 0, 1, 0, -1};
    static const int line_dy[8] = {-1, -1, -1, -1, 0, 0, 0, 0};
    static const int line_dz[8] = {0, 0, 0, 0, 1, 0, -1, 0};
    if (face < 0 || face >= 8) {
        return 0;
    }
    int count = 0;
    float max_width = 64;
    float line_height = 1.25;
    char lines[1024];
    int rows = wrap(text, max_width, lines, 1024);
    rows = MIN(rows, 5);
    int dx = glyph_dx[face];
    int dz = glyph_dz[face];
    int ldx = line_dx[face];
    int ldy = line_dy[face];
    int ldz = line_dz[face];
    float n = 1.0 / (max_width / 10);
    float sx = x - n * (rows - 1) * (line_height / 2) * ldx;
    float sy = y - n * (rows - 1) * (line_height / 2) * ldy;
    float sz = z - n * (rows - 1) * (line_height / 2) * ldz;
    char *key;
    char *line = tokenize(lines, "\n", &key);
    while (line) {
        int length = strlen(line);
        int line_width = string_width(line);
        line_width = MIN(line_width, max_width);
        float rx = sx - dx * line_width / max_width / 2;
        float ry = sy;
        float rz = sz - dz * line_width / max_width / 2;
        for (int i = 0; i < length; i++) {
            int width = char_width(line[i]);
            line_width -= width;
            if (line_width < 0) {
                break;
            }
            rx += dx * width / max_width / 2;
            rz += dz * width / max_width / 2;
            if (line[i] != ' ') {
                make_character_3d(
                    data + count * 30, rx, ry, rz, n / 2, face, line[i]);
                count++;
            }
            rx += dx * width / max_width / 2;
            rz += dz * width / max_width / 2;
        }
        sx += n * line_height * ldx;
        sy += n * line_height * ldy;
        sz += n * line_height * ldz;
        line = tokenize(NULL, "\n", &key);
        rows--;
        if (rows <= 0) {
            break;
        }
    }
    return count;
}

void gen_sign_buffer(Chunk *chunk) {
    SignList *signs = &chunk->signs;

    // first pass - count characters
    int max_faces = 0;
    for (int i = 0; i < signs->size; i++) {
        Sign *e = signs->data + i;
        max_faces += strlen(e->text);
    }

    // second pass - generate geometry
    GLfloat *data = malloc_faces(5, max_faces);
    int faces = 0;
    for (int i = 0; i < signs->size; i++) {
        Sign *e = signs->data + i;
        faces += _gen_sign_buffer(
            data + faces * 30, e->x, e->y, e->z, e->face, e->text);
    }

    del_buffer(chunk->sign_buffer);
    chunk->sign_buffer = gen_faces(5, faces, data);
    chunk->sign_faces = faces;
}

int has_lights(Chunk *chunk) {
    if (!SHOW_LIGHTS) {
        return 0;
    }
    for (int dp = -1; dp <= 1; dp++) {
        for (int dq = -1; dq <= 1; dq++) {
            Chunk *other = chunk;
            if (dp || dq) {
                other = find_chunk(chunk->p + dp, chunk->q + dq);
            }
            if (!other) {
                continue;
            }
            Map *map = &other->lights;
            if (map->size) {
                return 1;
            }
        }
    }
    return 0;
}

void dirty_chunk(Chunk *chunk) {
    chunk->dirty = 1;
    if (has_lights(chunk)) {
        for (int dp = -1; dp <= 1; dp++) {
            for (int dq = -1; dq <= 1; dq++) {
                Chunk *other = find_chunk(chunk->p + dp, chunk->q + dq);
                if (other) {
                    other->dirty = 1;
                }
            }
        }
    }
}

void occlusion(
    char neighbors[27], char lights[27], float shades[27],
    float ao[6][4], float light[6][4])
{
    static const int lookup3[6][4][3] = {
        {{0, 1, 3}, {2, 1, 5}, {6, 3, 7}, {8, 5, 7}},
        {{18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25}},
        {{6, 7, 15}, {8, 7, 17}, {24, 15, 25}, {26, 17, 25}},
        {{0, 1, 9}, {2, 1, 11}, {18, 9, 19}, {20, 11, 19}},
        {{0, 3, 9}, {6, 3, 15}, {18, 9, 21}, {24, 15, 21}},
        {{2, 5, 11}, {8, 5, 17}, {20, 11, 23}, {26, 17, 23}}
    };
   static const int lookup4[6][4][4] = {
        {{0, 1, 3, 4}, {1, 2, 4, 5}, {3, 4, 6, 7}, {4, 5, 7, 8}},
        {{18, 19, 21, 22}, {19, 20, 22, 23}, {21, 22, 24, 25}, {22, 23, 25, 26}},
        {{6, 7, 15, 16}, {7, 8, 16, 17}, {15, 16, 24, 25}, {16, 17, 25, 26}},
        {{0, 1, 9, 10}, {1, 2, 10, 11}, {9, 10, 18, 19}, {10, 11, 19, 20}},
        {{0, 3, 9, 12}, {3, 6, 12, 15}, {9, 12, 18, 21}, {12, 15, 21, 24}},
        {{2, 5, 11, 14}, {5, 8, 14, 17}, {11, 14, 20, 23}, {14, 17, 23, 26}}
    };
    static const float curve[4] = {0.0, 0.25, 0.5, 0.75};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            int corner = neighbors[lookup3[i][j][0]];
            int side1 = neighbors[lookup3[i][j][1]];
            int side2 = neighbors[lookup3[i][j][2]];
            int value = side1 && side2 ? 3 : corner + side1 + side2;
            float shade_sum = 0;
            float light_sum = 0;
            int is_light = lights[13] == 15;
            for (int k = 0; k < 4; k++) {
                shade_sum += shades[lookup4[i][j][k]];
                light_sum += lights[lookup4[i][j][k]];
            }
            if (is_light) {
                light_sum = 15 * 4 * 10;
            }
            float total = curve[value] + shade_sum / 4.0;
            ao[i][j] = MIN(total, 1.0);
            light[i][j] = light_sum / 15.0 / 4.0;
        }
    }
}

#define XZ_SIZE (CHUNK_SIZE * 3 + 2)
#define XZ_LO (CHUNK_SIZE)
#define XZ_HI (CHUNK_SIZE * 2 + 1)
#define Y_SIZE 258
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

void light_fill(
    char *opaque, char *light,
    int x, int y, int z, int w, int force)
{
    if (x + w < XZ_LO || z + w < XZ_LO) {
        return;
    }
    if (x - w > XZ_HI || z - w > XZ_HI) {
        return;
    }
    if (y < 0 || y >= Y_SIZE) {
        return;
    }
    if (light[XYZ(x, y, z)] >= w) {
        return;
    }
    if (!force && opaque[XYZ(x, y, z)]) {
        return;
    }
    light[XYZ(x, y, z)] = w--;
    light_fill(opaque, light, x - 1, y, z, w, 0);
    light_fill(opaque, light, x + 1, y, z, w, 0);
    light_fill(opaque, light, x, y - 1, z, w, 0);
    light_fill(opaque, light, x, y + 1, z, w, 0);
    light_fill(opaque, light, x, y, z - 1, w, 0);
    light_fill(opaque, light, x, y, z + 1, w, 0);
}

void compute_chunk(WorkerItem *item) {
    char *opaque = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
    char *light = (char *)calloc(XZ_SIZE * XZ_SIZE * Y_SIZE, sizeof(char));
    char *highest = (char *)calloc(XZ_SIZE * XZ_SIZE, sizeof(char));

    int ox = item->p * CHUNK_SIZE - CHUNK_SIZE - 1;
    int oy = -1;
    int oz = item->q * CHUNK_SIZE - CHUNK_SIZE - 1;

    // check for lights
    int has_light = 0;
    if (SHOW_LIGHTS) {
        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                Map *map = item->light_maps[a][b];
                if (map && map->size) {
                    has_light = 1;
                }
            }
        }
    }

    // populate opaque array
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            Map *map = item->block_maps[a][b];
            if (!map) {
                continue;
            }
            MAP_FOR_EACH(map, ex, ey, ez, ew) {
                int x = ex - ox;
                int y = ey - oy;
                int z = ez - oz;
                int w = ew;
                // TODO: this should be unnecessary
                if (x < 0 || y < 0 || z < 0) {
                    continue;
                }
                if (x >= XZ_SIZE || y >= Y_SIZE || z >= XZ_SIZE) {
                    continue;
                }
                // END TODO
                opaque[XYZ(x, y, z)] = !is_transparent(w);
                if (opaque[XYZ(x, y, z)]) {
                    highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
                }
            } END_MAP_FOR_EACH;
        }
    }

    // flood fill light intensities
    if (has_light) {
        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                Map *map = item->light_maps[a][b];
                if (!map) {
                    continue;
                }
                MAP_FOR_EACH(map, ex, ey, ez, ew) {
                    int x = ex - ox;
                    int y = ey - oy;
                    int z = ez - oz;
                    light_fill(opaque, light, x, y, z, ew, 1);
                } END_MAP_FOR_EACH;
            }
        }
    }

    Map *map = item->block_maps[1][1];

    // count exposed faces
    int miny = 256;
    int maxy = 0;
    int faces = 0;
    MAP_FOR_EACH(map, ex, ey, ez, ew) {
        if (ew <= 0) {
            continue;
        }
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - oz;
        int f1 = !opaque[XYZ(x - 1, y, z)];
        int f2 = !opaque[XYZ(x + 1, y, z)];
        int f3 = !opaque[XYZ(x, y + 1, z)];
        int f4 = !opaque[XYZ(x, y - 1, z)] && (ey > 0);
        int f5 = !opaque[XYZ(x, y, z - 1)];
        int f6 = !opaque[XYZ(x, y, z + 1)];
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        if (is_plant(ew)) {
            total = 4;
        }
        miny = MIN(miny, ey);
        maxy = MAX(maxy, ey);
        faces += total;
    } END_MAP_FOR_EACH;

    // generate geometry
    GLfloat *data = malloc_faces(10, faces);
    int offset = 0;
    MAP_FOR_EACH(map, ex, ey, ez, ew) {
        if (ew <= 0) {
            continue;
        }
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - oz;
        int f1 = !opaque[XYZ(x - 1, y, z)];
        int f2 = !opaque[XYZ(x + 1, y, z)];
        int f3 = !opaque[XYZ(x, y + 1, z)];
        int f4 = !opaque[XYZ(x, y - 1, z)] && (ey > 0);
        int f5 = !opaque[XYZ(x, y, z - 1)];
        int f6 = !opaque[XYZ(x, y, z + 1)];
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        char neighbors[27] = {0};
        char lights[27] = {0};
        float shades[27] = {0};
        int index = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -1; dz <= 1; dz++) {
                    neighbors[index] = opaque[XYZ(x + dx, y + dy, z + dz)];
                    lights[index] = light[XYZ(x + dx, y + dy, z + dz)];
                    shades[index] = 0;
                    if (y + dy <= highest[XZ(x + dx, z + dz)]) {
                        for (int oy = 0; oy < 8; oy++) {
                            if (opaque[XYZ(x + dx, y + dy + oy, z + dz)]) {
                                shades[index] = 1.0 - oy * 0.125;
                                break;
                            }
                        }
                    }
                    index++;
                }
            }
        }
        float ao[6][4];
        float light[6][4];
        occlusion(neighbors, lights, shades, ao, light);
        if (is_plant(ew)) {
            total = 4;
            float min_ao = 1;
            float max_light = 0;
            for (int a = 0; a < 6; a++) {
                for (int b = 0; b < 4; b++) {
                    min_ao = MIN(min_ao, ao[a][b]);
                    max_light = MAX(max_light, light[a][b]);
                }
            }
            float rotation = simplex2(ex, ez, 4, 0.5, 2) * 360;
            make_plant(
                data + offset, min_ao, max_light,
                ex, ey, ez, 0.5, ew, rotation);
        }
        else if (is_noncube(ew) && noncube_type(ew) == NonCubeType_SLAB_LOWER) {
            make_cube(
                data + offset, ao, light,
                f1, f2, f3, f4, f5, f6,
                ex, ey, ez, 0.5, ew);
        }
        else {
            make_cube(
                data + offset, ao, light,
                f1, f2, f3, f4, f5, f6,
                ex, ey, ez, 0.5, ew);
        }
        offset += total * 60;
    } END_MAP_FOR_EACH;

    free(opaque);
    free(light);
    free(highest);

    item->miny = miny;
    item->maxy = maxy;
    item->faces = faces;
    item->data = data;
}

void generate_chunk(Chunk *chunk, WorkerItem *item) {
    chunk->miny = item->miny;
    chunk->maxy = item->maxy;
    chunk->faces = item->faces;
    del_buffer(chunk->buffer);
    chunk->buffer = gen_faces(10, item->faces, item->data);
    gen_sign_buffer(chunk);
}

void gen_chunk_buffer(Chunk *chunk) {
    WorkerItem _item;
    WorkerItem *item = &_item;
    item->p = chunk->p;
    item->q = chunk->q;
    for (int dp = -1; dp <= 1; dp++) {
        for (int dq = -1; dq <= 1; dq++) {
            Chunk *other = chunk;
            if (dp || dq) {
                other = find_chunk(chunk->p + dp, chunk->q + dq);
            }
            if (other) {
                item->block_maps[dp + 1][dq + 1] = &other->map;
                item->light_maps[dp + 1][dq + 1] = &other->lights;
            }
            else {
                item->block_maps[dp + 1][dq + 1] = 0;
                item->light_maps[dp + 1][dq + 1] = 0;
            }
        }
    }
    compute_chunk(item);
    generate_chunk(chunk, item);
    chunk->dirty = 0;
}

void map_set_func(int x, int y, int z, int w, void *arg) {
    Map *map = (Map *)arg;
    map_set(map, x, y, z, w);
}

void load_chunk(WorkerItem *item) {
    int p = item->p;
    int q = item->q;
    Map *block_map = item->block_maps[1][1];
    Map *light_map = item->light_maps[1][1];
    create_world(p, q, map_set_func, block_map);
    db_load_blocks(block_map, p, q);
    db_load_lights(light_map, p, q);
}

void request_chunk(int p, int q) {
    int key = db_get_key(p, q);
    client_chunk(p, q, key);
}

void init_chunk(Chunk *chunk, int p, int q) {
    chunk->p = p;
    chunk->q = q;
    chunk->faces = 0;
    chunk->sign_faces = 0;
    chunk->buffer = 0;
    chunk->sign_buffer = 0;
    dirty_chunk(chunk);
    SignList *signs = &chunk->signs;
    sign_list_alloc(signs, 16);
    db_load_signs(signs, p, q);
    Map *block_map = &chunk->map;
    Map *light_map = &chunk->lights;
    int dx = p * CHUNK_SIZE - 1;
    int dy = 0;
    int dz = q * CHUNK_SIZE - 1;
    map_alloc(block_map, dx, dy, dz, 0x7fff);
    map_alloc(light_map, dx, dy, dz, 0xf);
}

void create_chunk(Chunk *chunk, int p, int q) {
    init_chunk(chunk, p, q);

    WorkerItem _item;
    WorkerItem *item = &_item;
    item->p = chunk->p;
    item->q = chunk->q;
    item->block_maps[1][1] = &chunk->map;
    item->light_maps[1][1] = &chunk->lights;
    load_chunk(item);

    request_chunk(p, q);
}

void delete_chunks() {
    int count = g->chunk_count;
    State *s1 = &g->players->state;
    State *s2 = &(g->players + g->observe1)->state;
    State *s3 = &(g->players + g->observe2)->state;
    State *states[3] = {s1, s2, s3};
    for (int i = 0; i < count; i++) {
        Chunk *chunk = g->chunks + i;
        int delete = 1;
        for (int j = 0; j < 3; j++) {
            State *s = states[j];
            int p = chunked(s->x);
            int q = chunked(s->z);
            if (chunk_distance(chunk, p, q) < g->delete_radius) {
                delete = 0;
                break;
            }
        }
        if (delete) {
            map_free(&chunk->map);
            map_free(&chunk->lights);
            sign_list_free(&chunk->signs);
            del_buffer(chunk->buffer);
            del_buffer(chunk->sign_buffer);
            Chunk *other = g->chunks + (--count);
            memcpy(chunk, other, sizeof(Chunk));
        }
    }
    g->chunk_count = count;
}

void delete_all_chunks() {
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        map_free(&chunk->map);
        map_free(&chunk->lights);
        sign_list_free(&chunk->signs);
        //printf("deleting: %d, %d\n", (int) chunk->buffer, (int) chunk->sign_buffer);
        del_buffer(chunk->buffer);
        del_buffer(chunk->sign_buffer);
    }
    g->chunk_count = 0;
}

void check_workers() {
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        mtx_lock(&worker->mtx);
        if (worker->state == WORKER_DONE) {
            WorkerItem *item = &worker->item;
            Chunk *chunk = find_chunk(item->p, item->q);
            if (chunk) {
                if (item->load) {
                    Map *block_map = item->block_maps[1][1];
                    Map *light_map = item->light_maps[1][1];
                    map_free(&chunk->map);
                    map_free(&chunk->lights);
                    map_copy(&chunk->map, block_map);
                    map_copy(&chunk->lights, light_map);
                    request_chunk(item->p, item->q);
                }
                generate_chunk(chunk, item);
            }
            for (int a = 0; a < 3; a++) {
                for (int b = 0; b < 3; b++) {
                    Map *block_map = item->block_maps[a][b];
                    Map *light_map = item->light_maps[a][b];
                    if (block_map) {
                        map_free(block_map);
                        free(block_map);
                    }
                    if (light_map) {
                        map_free(light_map);
                        free(light_map);
                    }
                }
            }
            worker->state = WORKER_IDLE;
        }
        mtx_unlock(&worker->mtx);
    }
}

void force_chunks(Player *player) {
    State *s = &player->state;
    int p = chunked(s->x);
    int q = chunked(s->z);
    int r = 1;
    for (int dp = -r; dp <= r; dp++) {
        for (int dq = -r; dq <= r; dq++) {
            int a = p + dp;
            int b = q + dq;
            Chunk *chunk = find_chunk(a, b);
            if (chunk) {
                if (chunk->dirty) {
                    gen_chunk_buffer(chunk);
                }
            }
            else if (g->chunk_count < MAX_CHUNKS) {
                chunk = g->chunks + g->chunk_count++;
                create_chunk(chunk, a, b);
                gen_chunk_buffer(chunk);
            }
        }
    }
}

void ensure_chunks_worker(Player *player, Worker *worker) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    float planes[6][4];
    frustum_planes(planes, g->render_radius, matrix);
    int p = chunked(s->x);
    int q = chunked(s->z);
    int r = g->create_radius;
    int start = 0x0fffffff;
    int best_score = start;
    int best_a = 0;
    int best_b = 0;
    for (int dp = -r; dp <= r; dp++) {
        for (int dq = -r; dq <= r; dq++) {
            int a = p + dp;
            int b = q + dq;
            int index = (ABS(a) ^ ABS(b)) % WORKERS;
            if (index != worker->index) {
                continue;
            }
            Chunk *chunk = find_chunk(a, b);
            if (chunk && !chunk->dirty) {
                continue;
            }
            int distance = MAX(ABS(dp), ABS(dq));
            int invisible = !chunk_visible(planes, a, b, 0, 256);
            int priority = 0;
            if (chunk) {
                priority = chunk->buffer && chunk->dirty;
            }
            int score = (invisible << 24) | (priority << 16) | distance;
            if (score < best_score) {
                best_score = score;
                best_a = a;
                best_b = b;
            }
        }
    }
    if (best_score == start) {
        return;
    }
    int a = best_a;
    int b = best_b;
    int load = 0;
    Chunk *chunk = find_chunk(a, b);
    if (!chunk) {
        load = 1;
        if (g->chunk_count < MAX_CHUNKS) {
            chunk = g->chunks + g->chunk_count++;
            init_chunk(chunk, a, b);
        }
        else {
            return;
        }
    }
    WorkerItem *item = &worker->item;
    item->p = chunk->p;
    item->q = chunk->q;
    item->load = load;
    for (int dp = -1; dp <= 1; dp++) {
        for (int dq = -1; dq <= 1; dq++) {
            Chunk *other = chunk;
            if (dp || dq) {
                other = find_chunk(chunk->p + dp, chunk->q + dq);
            }
            if (other) {
                Map *block_map = malloc(sizeof(Map));
                map_copy(block_map, &other->map);
                Map *light_map = malloc(sizeof(Map));
                map_copy(light_map, &other->lights);
                item->block_maps[dp + 1][dq + 1] = block_map;
                item->light_maps[dp + 1][dq + 1] = light_map;
            }
            else {
                item->block_maps[dp + 1][dq + 1] = 0;
                item->light_maps[dp + 1][dq + 1] = 0;
            }
        }
    }
    chunk->dirty = 0;
    worker->state = WORKER_BUSY;
    cnd_signal(&worker->cnd);
}

void ensure_chunks(Player *player) {
    check_workers();
    force_chunks(player);
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        mtx_lock(&worker->mtx);
        if (worker->state == WORKER_IDLE) {
            ensure_chunks_worker(player, worker);
        }
        mtx_unlock(&worker->mtx);
    }
}

int worker_run(void *arg) {
    Worker *worker = (Worker *)arg;
    int running = 1;
    while (running) {
        mtx_lock(&worker->mtx);
        while (worker->state != WORKER_BUSY) {
            cnd_wait(&worker->cnd, &worker->mtx);
        }
        mtx_unlock(&worker->mtx);
        WorkerItem *item = &worker->item;
        if (item->load) {
            load_chunk(item);
        }
        compute_chunk(item);
        mtx_lock(&worker->mtx);
        worker->state = WORKER_DONE;
        mtx_unlock(&worker->mtx);
    }
    return 0;
}

void unset_sign(int x, int y, int z) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        if (sign_list_remove_all(signs, x, y, z)) {
            chunk->dirty = 1;
            db_delete_signs(x, y, z);
        }
    }
    else {
        db_delete_signs(x, y, z);
    }
}

void unset_sign_face(int x, int y, int z, int face) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        if (sign_list_remove(signs, x, y, z, face)) {
            chunk->dirty = 1;
            db_delete_sign(x, y, z, face);
        }
    }
    else {
        db_delete_sign(x, y, z, face);
    }
}

void _set_sign(
    int p, int q, int x, int y, int z, int face, const char *text, int dirty)
{
    if (strlen(text) == 0) {
        unset_sign_face(x, y, z, face);
        return;
    }
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        sign_list_add(signs, x, y, z, face, text);
        if (dirty) {
            chunk->dirty = 1;
        }
    }
    db_insert_sign(p, q, x, y, z, face, text);
}

void set_sign(int x, int y, int z, int face, const char *text) {
    int p = chunked(x);
    int q = chunked(z);
    _set_sign(p, q, x, y, z, face, text, 1);
    client_sign(x, y, z, face, text);
}

void toggle_light(int x, int y, int z) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->lights;
        int w = map_get(map, x, y, z) ? 0 : 15;
        map_set(map, x, y, z, w);
        db_insert_light(p, q, x, y, z, w);
        client_light(x, y, z, w);
        dirty_chunk(chunk);
    }
}

void set_light(int p, int q, int x, int y, int z, int w) {
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->lights;
        if (map_set(map, x, y, z, w)) {
            dirty_chunk(chunk);
            db_insert_light(p, q, x, y, z, w);
        }
    }
    else {
        db_insert_light(p, q, x, y, z, w);
    }
}

void _set_block(int p, int q, int x, int y, int z, int w, int dirty) {
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        if (map_set(map, x, y, z, w)) {
            if (dirty) {
                dirty_chunk(chunk);
            }
            db_insert_block(p, q, x, y, z, w);
        }
    }
    else {
        db_insert_block(p, q, x, y, z, w);
    }
    if (w == 0 && chunked(x) == p && chunked(z) == q) {
        unset_sign(x, y, z);
        set_light(p, q, x, y, z, 0);
    }
}

void set_block(int x, int y, int z, int w) {
    int p = chunked(x);
    int q = chunked(z);
    _set_block(p, q, x, y, z, w, 1);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dz == 0) {
                continue;
            }
            if (dx && chunked(x + dx) == p) {
                continue;
            }
            if (dz && chunked(z + dz) == q) {
                continue;
            }
            _set_block(p + dx, q + dz, x, y, z, -w, 1);
        }
    }
    client_block(x, y, z, w);
}

void record_block(int x, int y, int z, int w) {
    memcpy(&g->block1, &g->block0, sizeof(Block));
    g->block0.x = x;
    g->block0.y = y;
    g->block0.z = z;
    g->block0.w = w;
}

int get_block(int x, int y, int z) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        return map_get(map, x, y, z);
    }
    return 0;
}

void builder_block(int x, int y, int z, int w) {
    if (y <= 0 || y >= BUILD_HEIGHT_LIMIT) {
        return;
    }
    if (is_destructable(get_block(x, y, z))) {
        set_block(x, y, z, 0);
    }
    if (w) {
        set_block(x, y, z, w);
    }
}

int render_chunks(Attrib *attrib, Player *player) {
    int result = 0;
    State *s = &player->state;
    ensure_chunks(player);
    int p = chunked(s->x);
    int q = chunked(s->z);
    float light = get_daylight();
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    float planes[6][4];
    frustum_planes(planes, g->render_radius, matrix);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
    glUniform1i(attrib->sampler, 0);
    glUniform1i(attrib->extra1, 2);
    glUniform1f(attrib->extra2, light);
    glUniform1f(attrib->extra3, g->render_radius * CHUNK_SIZE);
    glUniform1i(attrib->extra4, g->ortho);
    glUniform1f(attrib->timer, time_of_day());

    SkyColor *c = &g->sky_color;
    glUniform3f(attrib->extra5, c->r, c->g, c->b);

    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk_distance(chunk, p, q) > g->render_radius) {
            continue;
        }
        if (!chunk_visible(
            planes, chunk->p, chunk->q, chunk->miny, chunk->maxy))
        {
            continue;
        }
        draw_chunk(attrib, chunk);
        result += chunk->faces;
    }
    return result;
}

void render_signs(Attrib *attrib, Player *player) {
    State *s = &player->state;
    int p = chunked(s->x);
    int q = chunked(s->z);
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    float planes[6][4];
    frustum_planes(planes, g->render_radius, matrix);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 3);
    glUniform1i(attrib->extra1, 1);
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk_distance(chunk, p, q) > g->sign_radius) {
            continue;
        }
        if (!chunk_visible(
            planes, chunk->p, chunk->q, chunk->miny, chunk->maxy))
        {
            continue;
        }
        draw_signs(attrib, chunk);
    }
}

void render_sign(Attrib *attrib, Player *player) {
    if (!g->typing || g->typing_buffer[0] != CRAFT_KEY_SIGN) {
        return;
    }
    int x, y, z, face;
    if (!hit_test_face(player, &x, &y, &z, &face)) {
        return;
    }
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 3);
    glUniform1i(attrib->extra1, 1);
    char text[MAX_SIGN_LENGTH];
    strncpy(text, g->typing_buffer + 1, MAX_SIGN_LENGTH);
    text[MAX_SIGN_LENGTH - 1] = '\0';
    GLfloat *data = malloc_faces(5, strlen(text));
    int length = _gen_sign_buffer(data, x, y, z, face, text);
    GLuint buffer = gen_faces(5, length, data);
    draw_sign(attrib, buffer, length);
    del_buffer(buffer);
}

void render_players(Attrib *attrib, Player *player) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, time_of_day());
    for (int i = 0; i < g->player_count; i++) {
        Player *other = g->players + i;
        if (other != player) {
            draw_player(attrib, other);
        }
    }
}

void render_sky(Attrib *attrib, Player *player, GLuint buffer) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        0, 0, 0, s->rx, s->ry, g->fov, 0, g->render_radius);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 2);
    glUniform1f(attrib->timer, time_of_day());
    glUniform3f(attrib->extra1, 1.0f,  1.0f, 1.0f);

    SkyColor *c = &g->sky_color;
    glUniform3f(attrib->extra1, c->r, c->g, c->b);

    draw_triangles_3d(attrib, buffer, 512 * 3);
}

void render_wireframe(Attrib *attrib, Player *player) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, g->width, g->height,
        s->x, s->y, s->z, s->rx, s->ry, g->fov, g->ortho, g->render_radius);
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (is_obstacle(hw)) {
        glUseProgram(attrib->program);
        glLineWidth(1);
        glEnable(GL_COLOR_LOGIC_OP);
        glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
        GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.53);
        draw_lines(attrib, wireframe_buffer, 3, 24);
        del_buffer(wireframe_buffer);
        glDisable(GL_COLOR_LOGIC_OP);
    }
}

void render_crosshairs(Attrib *attrib) {
    float matrix[16];
    set_matrix_2d(matrix, g->width, g->height);
    glUseProgram(attrib->program);
    glLineWidth(4 * g->scale);
    glEnable(GL_COLOR_LOGIC_OP);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    GLuint crosshair_buffer = gen_crosshair_buffer();
    draw_lines(attrib, crosshair_buffer, 2, 4);
    del_buffer(crosshair_buffer);
    glDisable(GL_COLOR_LOGIC_OP);
}

void render_item(Attrib *attrib) {
    float matrix[16];
    glUseProgram(attrib->program);
    set_matrix_item(matrix, g->width, g->height, g->scale + 1);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, time_of_day());
    for(int i = 0; i < NUM_INVENTORY_VISIBLE; ++i) {
        if(g->item_index + i >= item_count) {
            break;
        }

        glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);

        int w = items[g->item_index + i];
        if (is_plant(w)) {
            GLuint buffer = gen_plant_buffer(0, 0, 0, 0.5, w);
            draw_plant(attrib, buffer);
            del_buffer(buffer);
        }
        else {
            GLuint buffer = gen_cube_buffer(0, 0, 0, 0.5, w);
            draw_cube(attrib, buffer);
            del_buffer(buffer);
        }

        if(!i) {
            set_matrix_item(matrix, g->width, g->height, g->scale);
            matrix[12] += 0.08f;
            matrix[13] += 0.1f;
        }

        matrix[13] += 0.25f;
    }
}

void render_text(
    Attrib *attrib, int justify, float x, float y, float n, char *text)
{
    float matrix[16];
    set_matrix_2d(matrix, g->width, g->height);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1);
    glUniform1i(attrib->extra1, 0);
    int length = strlen(text);
    x -= n * justify * (length - 1) / 2;
    GLuint buffer = gen_text_buffer(x, y, n, text);
    draw_text(attrib, buffer, length);
    del_buffer(buffer);
}

void render_item_count(Attrib *attrib, float ts) {
    const int buf_len = 4;

    float pos = 15.0f;
    for(int i = 0; i < NUM_INVENTORY_VISIBLE; ++i) {
        if(g->item_index + i >= item_count) {
            break;
        }

        char buf[buf_len];
        snprintf(buf, buf_len, "%d\n", Inventory_getCount(&g->inventory, items[g->item_index + i]));
        //snprintf(buf, buf_len, "%d\n", g->inventory.count[items[g->item_index]]);

        render_text(attrib, ALIGN_CENTER, g->width - 20.0f, pos, ts, buf);
        //printf("%d\n", g->width);

        float ratio_to_hardcoded = (g->height / 768.0f);
        if(i) {
            pos += 96.0f * ratio_to_hardcoded;
        } else {
            pos += 140.0f * ratio_to_hardcoded;
        }
    }
}

void add_message(const char *text) {
    printf("%s\n", text);
    snprintf(
        g->messages[g->message_index], MAX_TEXT_LENGTH, "%s", text);
    g->message_index = (g->message_index + 1) % MAX_MESSAGES;
}

void login() {
    char username[128] = {0};
    char identity_token[128] = {0};
    char access_token[128] = {0};
    if (db_auth_get_selected(username, 128, identity_token, 128)) {
        printf("Contacting login server for username: %s\n", username);
        if (get_access_token(
            access_token, 128, username, identity_token))
        {
            printf("Successfully authenticated with the login server\n");
            client_login(username, access_token);
        }
        else {
            printf("Failed to authenticate with the login server\n");
            client_login("", "");
        }
    }
    else {
        printf("Logging in anonymously\n");
        client_login("", "");
    }
}

void copy() {
    memcpy(&g->copy0, &g->block0, sizeof(Block));
    memcpy(&g->copy1, &g->block1, sizeof(Block));
}

void paste() {
    Block *c1 = &g->copy1;
    Block *c2 = &g->copy0;
    Block *p1 = &g->block1;
    Block *p2 = &g->block0;
    int scx = SIGN(c2->x - c1->x);
    int scz = SIGN(c2->z - c1->z);
    int spx = SIGN(p2->x - p1->x);
    int spz = SIGN(p2->z - p1->z);
    int oy = p1->y - c1->y;
    int dx = ABS(c2->x - c1->x);
    int dz = ABS(c2->z - c1->z);
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x <= dx; x++) {
            for (int z = 0; z <= dz; z++) {
                int w = get_block(c1->x + x * scx, y, c1->z + z * scz);
                builder_block(p1->x + x * spx, y + oy, p1->z + z * spz, w);
            }
        }
    }
}

void array(Block *b1, Block *b2, int xc, int yc, int zc) {
    if (b1->w != b2->w) {
        return;
    }
    int w = b1->w;
    int dx = b2->x - b1->x;
    int dy = b2->y - b1->y;
    int dz = b2->z - b1->z;
    xc = dx ? xc : 1;
    yc = dy ? yc : 1;
    zc = dz ? zc : 1;
    for (int i = 0; i < xc; i++) {
        int x = b1->x + dx * i;
        for (int j = 0; j < yc; j++) {
            int y = b1->y + dy * j;
            for (int k = 0; k < zc; k++) {
                int z = b1->z + dz * k;
                builder_block(x, y, z, w);
            }
        }
    }
}

void cube(Block *b1, Block *b2, int fill) {
    if (b1->w != b2->w) {
        return;
    }
    int w = b1->w;
    int x1 = MIN(b1->x, b2->x);
    int y1 = MIN(b1->y, b2->y);
    int z1 = MIN(b1->z, b2->z);
    int x2 = MAX(b1->x, b2->x);
    int y2 = MAX(b1->y, b2->y);
    int z2 = MAX(b1->z, b2->z);
    int a = (x1 == x2) + (y1 == y2) + (z1 == z2);
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            for (int z = z1; z <= z2; z++) {
                if (!fill) {
                    int n = 0;
                    n += x == x1 || x == x2;
                    n += y == y1 || y == y2;
                    n += z == z1 || z == z2;
                    if (n <= a) {
                        continue;
                    }
                }
                builder_block(x, y, z, w);
            }
        }
    }
}

void sphere(Block *center, int radius, int fill, int fx, int fy, int fz) {
    static const float offsets[8][3] = {
        {-0.5, -0.5, -0.5},
        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, -0.5},
        {-0.5, 0.5, 0.5},
        {0.5, -0.5, -0.5},
        {0.5, -0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, 0.5, 0.5}
    };
    int cx = center->x;
    int cy = center->y;
    int cz = center->z;
    int w = center->w;
    for (int x = cx - radius; x <= cx + radius; x++) {
        if (fx && x != cx) {
            continue;
        }
        for (int y = cy - radius; y <= cy + radius; y++) {
            if (fy && y != cy) {
                continue;
            }
            for (int z = cz - radius; z <= cz + radius; z++) {
                if (fz && z != cz) {
                    continue;
                }
                int inside = 0;
                int outside = fill;
                for (int i = 0; i < 8; i++) {
                    float dx = x + offsets[i][0] - cx;
                    float dy = y + offsets[i][1] - cy;
                    float dz = z + offsets[i][2] - cz;
                    float d = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (d < radius) {
                        inside = 1;
                    }
                    else {
                        outside = 1;
                    }
                }
                if (inside && outside) {
                    builder_block(x, y, z, w);
                }
            }
        }
    }
}

void cylinder(Block *b1, Block *b2, int radius, int fill) {
    if (b1->w != b2->w) {
        return;
    }
    int w = b1->w;
    int x1 = MIN(b1->x, b2->x);
    int y1 = MIN(b1->y, b2->y);
    int z1 = MIN(b1->z, b2->z);
    int x2 = MAX(b1->x, b2->x);
    int y2 = MAX(b1->y, b2->y);
    int z2 = MAX(b1->z, b2->z);
    int fx = x1 != x2;
    int fy = y1 != y2;
    int fz = z1 != z2;
    if (fx + fy + fz != 1) {
        return;
    }
    Block block = {x1, y1, z1, w};
    if (fx) {
        for (int x = x1; x <= x2; x++) {
            block.x = x;
            sphere(&block, radius, fill, 1, 0, 0);
        }
    }
    if (fy) {
        for (int y = y1; y <= y2; y++) {
            block.y = y;
            sphere(&block, radius, fill, 0, 1, 0);
        }
    }
    if (fz) {
        for (int z = z1; z <= z2; z++) {
            block.z = z;
            sphere(&block, radius, fill, 0, 0, 1);
        }
    }
}

void tree(Block *block) {
    int bx = block->x;
    int by = block->y;
    int bz = block->z;
    for (int y = by + 3; y < by + 8; y++) {
        for (int dx = -3; dx <= 3; dx++) {
            for (int dz = -3; dz <= 3; dz++) {
                int dy = y - (by + 4);
                int d = (dx * dx) + (dy * dy) + (dz * dz);
                if (d < 11) {
                    builder_block(bx + dx, y, bz + dz, 15);
                }
            }
        }
    }
    for (int y = by; y < by + 7; y++) {
        builder_block(bx, y, bz, 5);
    }
}

void parse_command(const char *buffer, int forward) {
    char username[128] = {0};
    char token[128] = {0};
    char server_addr[MAX_ADDR_LENGTH];
    int server_port = DEFAULT_PORT;
    char filename[MAX_PATH_LENGTH];
    int radius, count, xc, yc, zc;
    if (sscanf(buffer, "/identity %128s %128s", username, token) == 2) {
        db_auth_set(username, token);
        add_message("Successfully imported identity token!");
        login();
    }
    else if (strcmp(buffer, "/logout") == 0) {
        db_auth_select_none();
        login();
    }
    else if (sscanf(buffer, "/login %128s", username) == 1) {
        if (db_auth_select(username)) {
            login();
        }
        else {
            add_message("Unknown username.");
        }
    }
    else if (sscanf(buffer,
        "/online %128s %d", server_addr, &server_port) >= 1)
    {
        g->mode_changed = 1;
        g->mode = MODE_ONLINE;
        strncpy(g->server_addr, server_addr, MAX_ADDR_LENGTH);
        g->server_port = server_port;
        snprintf(g->db_path, MAX_PATH_LENGTH,
            "cache.%s.%d.db", g->server_addr, g->server_port);
    }
    else if (sscanf(buffer, "/offline %128s", filename) == 1) {
        g->mode_changed = 1;
        g->mode = MODE_OFFLINE;
        snprintf(g->db_path, MAX_PATH_LENGTH, "%s.db", filename);
    }
    else if (strcmp(buffer, "/offline") == 0) {
        g->mode_changed = 1;
        g->mode = MODE_OFFLINE;
        snprintf(g->db_path, MAX_PATH_LENGTH, "%s", DB_PATH);
    }
    else if (sscanf(buffer, "/view %d", &radius) == 1) {
        if (radius >= 1 && radius <= 24) {
            g->create_radius = radius;
            g->render_radius = radius;
            g->delete_radius = radius + 4;
        }
        else {
            add_message("Viewing distance must be between 1 and 24.");
        }
    }
    else if (strcmp(buffer, "/copy") == 0) {
        copy();
    }
    else if (strcmp(buffer, "/paste") == 0) {
        paste();
    }
    else if (strcmp(buffer, "/tree") == 0) {
        tree(&g->block0);
    }
    else if (sscanf(buffer, "/array %d %d %d", &xc, &yc, &zc) == 3) {
        array(&g->block1, &g->block0, xc, yc, zc);
    }
    else if (sscanf(buffer, "/array %d", &count) == 1) {
        array(&g->block1, &g->block0, count, count, count);
    }
    else if (strcmp(buffer, "/fcube") == 0) {
        cube(&g->block0, &g->block1, 1);
    }
    else if (strcmp(buffer, "/cube") == 0) {
        cube(&g->block0, &g->block1, 0);
    }
    else if (sscanf(buffer, "/fsphere %d", &radius) == 1) {
        sphere(&g->block0, radius, 1, 0, 0, 0);
    }
    else if (sscanf(buffer, "/sphere %d", &radius) == 1) {
        sphere(&g->block0, radius, 0, 0, 0, 0);
    }
    else if (sscanf(buffer, "/fcirclex %d", &radius) == 1) {
        sphere(&g->block0, radius, 1, 1, 0, 0);
    }
    else if (sscanf(buffer, "/circlex %d", &radius) == 1) {
        sphere(&g->block0, radius, 0, 1, 0, 0);
    }
    else if (sscanf(buffer, "/fcircley %d", &radius) == 1) {
        sphere(&g->block0, radius, 1, 0, 1, 0);
    }
    else if (sscanf(buffer, "/circley %d", &radius) == 1) {
        sphere(&g->block0, radius, 0, 0, 1, 0);
    }
    else if (sscanf(buffer, "/fcirclez %d", &radius) == 1) {
        sphere(&g->block0, radius, 1, 0, 0, 1);
    }
    else if (sscanf(buffer, "/circlez %d", &radius) == 1) {
        sphere(&g->block0, radius, 0, 0, 0, 1);
    }
    else if (sscanf(buffer, "/fcylinder %d", &radius) == 1) {
        cylinder(&g->block0, &g->block1, radius, 1);
    }
    else if (sscanf(buffer, "/cylinder %d", &radius) == 1) {
        cylinder(&g->block0, &g->block1, radius, 0);
    }
    else if (forward) {
        client_talk(buffer);
    }
}

void on_light() {
    State *s = &g->players->state;
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (hy > 0 && hy < BUILD_HEIGHT_LIMIT && is_destructable(hw)) {
        toggle_light(hx, hy, hz);
    }
}

void on_left_click() {
    State *s = &g->players->state;
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (hy > 0 && hy < BUILD_HEIGHT_LIMIT && is_destructable(hw) && Inventory_collect(&g->inventory, hw)) {
        set_block(hx, hy, hz, 0);
        record_block(hx, hy, hz, 0);
        if (is_plant(get_block(hx, hy + 1, hz))) {
            set_block(hx, hy + 1, hz, 0);
        }
    }
}

void on_right_click() {
    State *s = &g->players->state;
    int hx, hy, hz;
    int hw = hit_test(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (hy > 0 && hy < BUILD_HEIGHT_LIMIT && is_obstacle(hw) && Inventory_use(&g->inventory, items[g->item_index])) {
        if (!player_intersects_block(2, s->x, s->y, s->z, hx, hy, hz)) {
            set_block(hx, hy, hz, items[g->item_index]);
            record_block(hx, hy, hz, items[g->item_index]);
        }
    }
}

void on_middle_click() {
    State *s = &g->players->state;
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    for (int i = 0; i < item_count; i++) {
        if (items[i] == hw) {
            g->item_index = i;
            break;
        }
    }
}

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
    int exclusive =
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    if (action == GLFW_RELEASE) {
        return;
    }
    if (key == GLFW_KEY_BACKSPACE) {
        if (g->typing) {
            int n = strlen(g->typing_buffer);
            if (n > 0) {
                g->typing_buffer[n - 1] = '\0';
            }
        }
    }
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        if (g->typing) {
            g->typing = 0;
        }
        else if (exclusive) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_ENTER) {
        if (g->typing) {
            if (mods & GLFW_MOD_SHIFT) {
                int n = strlen(g->typing_buffer);
                if (n < MAX_TEXT_LENGTH - 1) {
                    g->typing_buffer[n] = '\r';
                    g->typing_buffer[n + 1] = '\0';
                }
            }
            else {
                g->typing = 0;
                if (g->typing_buffer[0] == CRAFT_KEY_SIGN) {
                    Player *player = g->players;
                    int x, y, z, face;
                    if (hit_test_face(player, &x, &y, &z, &face)) {
                        set_sign(x, y, z, face, g->typing_buffer + 1);
                    }
                }
                else if (g->typing_buffer[0] == '/') {
                    parse_command(g->typing_buffer, 1);
                }
                else {
                    client_talk(g->typing_buffer);
                }
            }
        }
        else {
            if (control) {
                on_right_click();
            }
            else {
                on_left_click();
            }
        }
    }
    if (control && key == 'V') {
        const char *buffer = glfwGetClipboardString(window);
        if (g->typing) {
            g->suppress_char = 1;
            strncat(g->typing_buffer, buffer,
                MAX_TEXT_LENGTH - strlen(g->typing_buffer) - 1);
        }
        else {
            parse_command(buffer, 0);
        }
    }
    if (!g->typing) {
        if (key == CRAFT_KEY_FLY) {
            g->flying = !g->flying;
        }
        if (key >= '1' && key <= '9') {
            g->item_index = key - '1';
        }
        if (key == '0') {
            g->item_index = 9;
        }
        if (key == CRAFT_KEY_ITEM_NEXT) {
            g->item_index = (g->item_index + 1) % item_count;
        }
        if (key == CRAFT_KEY_ITEM_PREV) {
            g->item_index--;
            if (g->item_index < 0) {
                g->item_index = item_count - 1;
            }
        }
        if (key == CRAFT_KEY_OBSERVE) {
            g->observe1 = (g->observe1 + 1) % g->player_count;
        }
        if (key == CRAFT_KEY_OBSERVE_INSET) {
            g->observe2 = (g->observe2 + 1) % g->player_count;
        }
    }

    /*
    if(key == GLFW_KEY_F11) {
        if(g->is_fullscreen) {
            GLFWmonitor *monitor = NULL;
            int mode_count;
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *modes = glfwGetVideoModes(monitor, &mode_count);
            int width = modes[mode_count - 1].width;
            int height = modes[mode_count - 1].height;

            glfwSetWindowMonitor(g->window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
        } else {
            glfwSetWindowMonitor(g->window, NULL, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GLFW_DONT_CARE);
        }
    }
    */
}

void on_char(GLFWwindow *window, unsigned int u) {
    if (g->suppress_char) {
        g->suppress_char = 0;
        return;
    }
    if (g->typing) {
        if (u >= 32 && u < 128) {
            char c = (char)u;
            int n = strlen(g->typing_buffer);
            if (n < MAX_TEXT_LENGTH - 1) {
                g->typing_buffer[n] = c;
                g->typing_buffer[n + 1] = '\0';
            }
        }
    }
    else {
        if (u == CRAFT_KEY_CHAT) {
            g->typing = 1;
            g->typing_buffer[0] = '\0';
        }
        if (u == CRAFT_KEY_COMMAND) {
            g->typing = 1;
            g->typing_buffer[0] = '/';
            g->typing_buffer[1] = '\0';
        }
        if (u == CRAFT_KEY_SIGN) {
            g->typing = 1;
            g->typing_buffer[0] = CRAFT_KEY_SIGN;
            g->typing_buffer[1] = '\0';
        }
    }
}

void on_scroll(GLFWwindow *window, double xdelta, double ydelta) {
    static double ypos = 0;
    ypos += ydelta;
    if (ypos < -SCROLL_THRESHOLD) {
        g->item_index = (g->item_index + 1) % item_count;
        ypos = 0;
    }
    if (ypos > SCROLL_THRESHOLD) {
        g->item_index--;
        if (g->item_index < 0) {
            g->item_index = item_count - 1;
        }
        ypos = 0;
    }
}

void on_mouse_button(GLFWwindow *window, int button, int action, int mods) {
    int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
    int exclusive =
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    if (action != GLFW_PRESS) {
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (exclusive) {
            if (control) {
                on_right_click();
            }
            else {
                on_left_click();
            }
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (exclusive) {
            if (control) {
                on_light();
            }
            else {
                on_right_click();
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (exclusive) {
            on_middle_click();
        }
    }
}

void create_window() {
    int window_width = WINDOW_WIDTH;
    int window_height = WINDOW_HEIGHT;
    GLFWmonitor *monitor = NULL;
    if (FULLSCREEN) {
        int mode_count;
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *modes = glfwGetVideoModes(monitor, &mode_count);
        window_width = modes[mode_count - 1].width;
        window_height = modes[mode_count - 1].height;
    }
    g->window = glfwCreateWindow(
        window_width, window_height, "Craft", monitor, NULL);

    //g->is_fullscreen = FULLSCREEN;
}

void handle_mouse_input() {
    int exclusive =
        glfwGetInputMode(g->window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    static double px = 0;
    static double py = 0;
    State *s = &g->players->state;
    if (exclusive && (px || py)) {
        double mx, my;
        glfwGetCursorPos(g->window, &mx, &my);
        float m = 0.0025;
        s->rx += (mx - px) * m;
        if (INVERT_MOUSE) {
            s->ry += (my - py) * m;
        }
        else {
            s->ry -= (my - py) * m;
        }
        if (s->rx < 0) {
            s->rx += RADIANS(360);
        }
        if (s->rx >= RADIANS(360)){
            s->rx -= RADIANS(360);
        }
        s->ry = MAX(s->ry, -RADIANS(90));
        s->ry = MIN(s->ry, RADIANS(90));
        px = mx;
        py = my;
    }
    else {
        glfwGetCursorPos(g->window, &px, &py);
    }
}

void handle_movement(double dt) {
    static float dy = 0;
    State *s = &g->players->state;
    int sz = 0;
    int sx = 0;
    if (!g->typing) {
        float m = dt * 1.0;
        g->ortho = glfwGetKey(g->window, CRAFT_KEY_ORTHO) ? 64 : 0;
        g->fov = glfwGetKey(g->window, CRAFT_KEY_ZOOM) ? 15 : 65;
        if (glfwGetKey(g->window, CRAFT_KEY_FORWARD)) sz--;
        if (glfwGetKey(g->window, CRAFT_KEY_BACKWARD)) sz++;
        if (glfwGetKey(g->window, CRAFT_KEY_LEFT)) sx--;
        if (glfwGetKey(g->window, CRAFT_KEY_RIGHT)) sx++;
        if (glfwGetKey(g->window, GLFW_KEY_LEFT)) s->rx -= m;
        if (glfwGetKey(g->window, GLFW_KEY_RIGHT)) s->rx += m;
        if (glfwGetKey(g->window, GLFW_KEY_UP)) s->ry += m;
        if (glfwGetKey(g->window, GLFW_KEY_DOWN)) s->ry -= m;
    }
    float vx, vy, vz;
    get_motion_vector(g->flying, sz, sx, s->rx, s->ry, &vx, &vy, &vz);
    int w = get_block(s->x, s->y, s->z);
    int is_jump_pressed = glfwGetKey(g->window, CRAFT_KEY_JUMP);
    int is_descend_pressed = glfwGetKey(g->window, CRAFT_KEY_DECSEND);
    if (!g->typing) {
        if (is_jump_pressed) {
            if (g->flying) {
                vy = 1;
            }
            else if (dy == 0) {
                if (is_climbable(w)) {
                    dy = CLIMB_SPEED;
                } else {
                    dy = 8;
                }
            }
        }
        else if (is_descend_pressed) {
            if (is_climbable(w)) {
                dy = -CLIMB_SPEED;
            }
        }
    }
    float speed = g->flying ? FLY_SPEED : WALK_SPEED;
    int estimate = roundf(sqrtf(
        powf(vx * speed, 2) +
        powf(vy * speed + ABS(dy) * 2, 2) +
        powf(vz * speed, 2)) * dt * 8);
    int step = MAX(8, estimate);
    float ut = dt / step;
    vx = vx * ut * speed;
    vy = vy * ut * speed;
    vz = vz * ut * speed;
    for (int i = 0; i < step; i++) {
        if (g->flying) {
            dy = 0;

            if (is_descend_pressed) {
                vy = -0.05f;
            }
        }
        else {
            if (is_climbable(w)) {
                if (!is_jump_pressed) {
                    if (dy > 0) {
                        dy = 0.0f;
                    }
                    else if (!is_descend_pressed) {
                        dy = 0.0f;
                    }
                }
            }
            else {
                dy -= ut * 25;
                dy = MAX(dy, -250);
            }
        }
        s->x += vx;
        s->y += vy + dy * ut;
        s->z += vz;
        if (collide(2, &s->x, &s->y, &s->z)) {
            dy = 0;
        }
    }
    if (s->y < 0) {
        s->y = highest_block(s->x, s->z) + 2;
    }
}

void parse_buffer(char *buffer) {
    Player *me = g->players;
    State *s = &g->players->state;
    char *key;
    char *line = tokenize(buffer, "\n", &key);
    while (line) {
        int pid;
        float ux, uy, uz, urx, ury;
        if (sscanf(line, "U,%d,%f,%f,%f,%f,%f",
            &pid, &ux, &uy, &uz, &urx, &ury) == 6)
        {
            me->id = pid;
            s->x = ux; s->y = uy; s->z = uz; s->rx = urx; s->ry = ury;
            force_chunks(me);
            if (uy == 0) {
                s->y = highest_block(s->x, s->z) + 2;
            }
        }
        int bp, bq, bx, by, bz, bw;
        if (sscanf(line, "B,%d,%d,%d,%d,%d,%d",
            &bp, &bq, &bx, &by, &bz, &bw) == 6)
        {
            _set_block(bp, bq, bx, by, bz, bw, 0);
            if (player_intersects_block(2, s->x, s->y, s->z, bx, by, bz)) {
                s->y = highest_block(s->x, s->z) + 2;
            }
        }
        if (sscanf(line, "L,%d,%d,%d,%d,%d,%d",
            &bp, &bq, &bx, &by, &bz, &bw) == 6)
        {
            set_light(bp, bq, bx, by, bz, bw);
        }
        float px, py, pz, prx, pry;
        if (sscanf(line, "P,%d,%f,%f,%f,%f,%f",
            &pid, &px, &py, &pz, &prx, &pry) == 6)
        {
            Player *player = find_player(pid);
            if (!player && g->player_count < MAX_PLAYERS) {
                player = g->players + g->player_count;
                g->player_count++;
                player->id = pid;
                player->buffer = 0;
                snprintf(player->name, MAX_NAME_LENGTH, "player%d", pid);
                update_player(player, px, py, pz, prx, pry, 1); // twice
            }
            if (player) {
                update_player(player, px, py, pz, prx, pry, 1);
            }
        }
        if (sscanf(line, "D,%d", &pid) == 1) {
            delete_player(pid);
        }
        int kp, kq, kk;
        if (sscanf(line, "K,%d,%d,%d", &kp, &kq, &kk) == 3) {
            db_set_key(kp, kq, kk);
        }
        if (sscanf(line, "R,%d,%d", &kp, &kq) == 2) {
            Chunk *chunk = find_chunk(kp, kq);
            if (chunk) {
                dirty_chunk(chunk);
            }
        }
        double elapsed;
        int day_length;
        if (sscanf(line, "E,%lf,%d", &elapsed, &day_length) == 2) {
            glfwSetTime(fmod(elapsed, day_length));
            g->day_length = day_length;
            g->time_changed = 1;
        }
        if (line[0] == 'T' && line[1] == ',') {
            char *text = line + 2;
            add_message(text);
        }
        char format[64];
        snprintf(
            format, sizeof(format), "N,%%d,%%%ds", MAX_NAME_LENGTH - 1);
        char name[MAX_NAME_LENGTH];
        if (sscanf(line, format, &pid, name) == 2) {
            Player *player = find_player(pid);
            if (player) {
                strncpy(player->name, name, MAX_NAME_LENGTH);
            }
        }
        snprintf(
            format, sizeof(format),
            "S,%%d,%%d,%%d,%%d,%%d,%%d,%%%d[^\n]", MAX_SIGN_LENGTH - 1);
        int face;
        char text[MAX_SIGN_LENGTH] = {0};
        if (sscanf(line, format,
            &bp, &bq, &bx, &by, &bz, &face, text) >= 6)
        {
            _set_sign(bp, bq, bx, by, bz, face, text, 0);
        }
        line = tokenize(NULL, "\n", &key);
    }
}

void update_sky_tint() {
    SkyColor *c = &g->sky_color;
    g->last_sky_color = *c;

    State *s = &g->players->state;
    int p = chunked(s->x);
    int q = chunked(s->z);
    int x = p * CHUNK_SIZE;
    int z = q * CHUNK_SIZE;

    SkyColor new_color;
    get_sky_tint(biome_at_pos(q, x, z), &new_color);

    //Interpolate sky color to get close to the actual value gradually.
    // Technically, it won't arrive, but rather achieve an extremely close color.
    c->r += 0.0625 * (new_color.r - c->r);
    c->g += 0.0625 * (new_color.g - c->g);
    c->b += 0.0625 * (new_color.b - c->b);

    //printf("c: %f last: %f\n", c->r, g->last_sky_color.r);
}

void reset_model() {
    memset(g->chunks, 0, sizeof(Chunk) * MAX_CHUNKS);
    g->chunk_count = 0;
    memset(g->players, 0, sizeof(Player) * MAX_PLAYERS);
    g->player_count = 0;
    g->observe1 = 0;
    g->observe2 = 0;
    g->flying = 0;
    g->item_index = 0;
    memset(g->typing_buffer, 0, sizeof(char) * MAX_TEXT_LENGTH);
    g->typing = 0;
    memset(g->messages, 0, sizeof(char) * MAX_MESSAGES * MAX_TEXT_LENGTH);
    g->message_index = 0;
    g->day_length = DAY_LENGTH;
    glfwSetTime(g->day_length / 3.0);
    g->time_changed = 1;

    g->sky_color = (SkyColor){0.0f, 0.0f, 0.0f};
    g->last_sky_color = g->sky_color;
    Inventory_reset(&g->inventory);
}

int main(int argc, char **argv) {
    // INITIALIZATION //
    curl_global_init(CURL_GLOBAL_DEFAULT);
    srand(time(NULL));
    rand();

    // WINDOW INITIALIZATION //
    if (!glfwInit()) {
        return -1;
    }
    create_window();
    if (!g->window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g->window);
    glfwSwapInterval(VSYNC);
    glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(g->window, on_key);
    glfwSetCharCallback(g->window, on_char);
    glfwSetMouseButtonCallback(g->window, on_mouse_button);
    glfwSetScrollCallback(g->window, on_scroll);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glLogicOp(GL_INVERT);
    glClearColor(0, 0, 0, 1);

    //parser_parse_all();

    // LOAD TEXTURES //
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("textures/texture.png");

    GLuint font;
    glGenTextures(1, &font);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    load_png_texture("textures/font.png");

    GLuint sky;
    glGenTextures(1, &sky);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, sky);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    load_png_texture("textures/sky.png");

    GLuint sign;
    glGenTextures(1, &sign);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sign);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("textures/sign.png");

    // LOAD SHADERS //
    Attrib block_attrib = {0};
    Attrib line_attrib = {0};
    Attrib text_attrib = {0};
    Attrib sky_attrib = {0};
    GLuint program;

    program = load_program(
        "shaders/block_vertex.glsl", "shaders/block_fragment.glsl");
    block_attrib.program = program;
    block_attrib.position = glGetAttribLocation(program, "position");
    block_attrib.normal = glGetAttribLocation(program, "normal");
    block_attrib.uv = glGetAttribLocation(program, "uv");
    block_attrib.matrix = glGetUniformLocation(program, "matrix");
    block_attrib.sampler = glGetUniformLocation(program, "sampler");
    block_attrib.extra1 = glGetUniformLocation(program, "sky_sampler");
    block_attrib.extra2 = glGetUniformLocation(program, "daylight");
    block_attrib.extra3 = glGetUniformLocation(program, "fog_distance");
    block_attrib.extra4 = glGetUniformLocation(program, "ortho");
    block_attrib.camera = glGetUniformLocation(program, "camera");
    block_attrib.timer = glGetUniformLocation(program, "timer");
    block_attrib.extra5 = glGetUniformLocation(program, "sky_tint");

    program = load_program(
        "shaders/line_vertex.glsl", "shaders/line_fragment.glsl");
    line_attrib.program = program;
    line_attrib.position = glGetAttribLocation(program, "position");
    line_attrib.matrix = glGetUniformLocation(program, "matrix");

    program = load_program(
        "shaders/text_vertex.glsl", "shaders/text_fragment.glsl");
    text_attrib.program = program;
    text_attrib.position = glGetAttribLocation(program, "position");
    text_attrib.uv = glGetAttribLocation(program, "uv");
    text_attrib.matrix = glGetUniformLocation(program, "matrix");
    text_attrib.sampler = glGetUniformLocation(program, "sampler");
    text_attrib.extra1 = glGetUniformLocation(program, "is_sign");

    program = load_program(
        "shaders/sky_vertex.glsl", "shaders/sky_fragment.glsl");
    sky_attrib.program = program;
    sky_attrib.position = glGetAttribLocation(program, "position");
    sky_attrib.normal = glGetAttribLocation(program, "normal");
    sky_attrib.uv = glGetAttribLocation(program, "uv");
    sky_attrib.matrix = glGetUniformLocation(program, "matrix");
    sky_attrib.sampler = glGetUniformLocation(program, "sampler");
    sky_attrib.timer = glGetUniformLocation(program, "timer");
    sky_attrib.extra1 = glGetUniformLocation(program, "sky_tint");

    // CHECK COMMAND LINE ARGUMENTS //
    if (argc == 2 || argc == 3) {
        g->mode = MODE_ONLINE;
        strncpy(g->server_addr, argv[1], MAX_ADDR_LENGTH);
        g->server_port = argc == 3 ? atoi(argv[2]) : DEFAULT_PORT;
        snprintf(g->db_path, MAX_PATH_LENGTH,
            "cache.%s.%d.db", g->server_addr, g->server_port);
    }
    else {
        g->mode = MODE_OFFLINE;
        snprintf(g->db_path, MAX_PATH_LENGTH, "%s", DB_PATH);
    }

    g->create_radius = CREATE_CHUNK_RADIUS;
    g->render_radius = RENDER_CHUNK_RADIUS;
    g->delete_radius = DELETE_CHUNK_RADIUS;
    g->sign_radius = RENDER_SIGN_RADIUS;

    // INITIALIZE WORKER THREADS
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        worker->index = i;
        worker->state = WORKER_IDLE;
        mtx_init(&worker->mtx, mtx_plain);
        cnd_init(&worker->cnd);
        thrd_create(&worker->thrd, worker_run, worker);
    }

    // OUTER LOOP //
    int running = 1;
    while (running) {
        // DATABASE INITIALIZATION //
        if (g->mode == MODE_OFFLINE || USE_CACHE) {
            db_enable();
            if (db_init(g->db_path)) {
                return -1;
            }
            if (g->mode == MODE_ONLINE) {
                // TODO: support proper caching of signs (handle deletions)
                db_delete_all_signs();
            }
        }

        // CLIENT INITIALIZATION //
        if (g->mode == MODE_ONLINE) {
            client_enable();
            client_connect(g->server_addr, g->server_port);
            client_start();
            client_version(1);
            login();
        }

        // LOCAL VARIABLES //
        reset_model();
        FPS fps = {0, 0, 0};
        double last_commit = glfwGetTime();
        double last_update = glfwGetTime();
        GLuint sky_buffer = gen_sky_buffer();

        Player *me = g->players;
        State *s = &g->players->state;
        me->id = 0;
        me->name[0] = '\0';
        me->buffer = 0;
        g->player_count = 1;

        // LOAD STATE FROM DATABASE //
        int loaded = db_load_state(&s->x, &s->y, &s->z, &s->rx, &s->ry);
        force_chunks(me);
        if (!loaded) {
            s->y = highest_block(s->x, s->z) + 2;
        }

        // BEGIN MAIN LOOP //
        double previous = glfwGetTime();
        while (1) {
            g->inventory.count[items[g->item_index]] = 16;
            //printf("%d %d\n", items[g->item_index], (int) g->inventory.count[items[g->item_index]]);

            // WINDOW SIZE AND SCALE //
            g->scale = get_scale_factor();
            glfwGetFramebufferSize(g->window, &g->width, &g->height);
            glViewport(0, 0, g->width, g->height);

            // FRAME RATE //
            if (g->time_changed) {
                g->time_changed = 0;
                last_commit = glfwGetTime();
                last_update = glfwGetTime();
                memset(&fps, 0, sizeof(fps));
            }
            update_fps(&fps);
            double now = glfwGetTime();
            double dt = now - previous;
            dt = MIN(dt, 0.2);
            dt = MAX(dt, 0.0);
            previous = now;

            // HANDLE MOUSE INPUT //
            handle_mouse_input();

            // HANDLE MOVEMENT //
            handle_movement(dt);

            // HANDLE DATA FROM SERVER //
            char *buffer = client_recv();
            if (buffer) {
                parse_buffer(buffer);
                free(buffer);
            }

            // FLUSH DATABASE //
            if (now - last_commit > COMMIT_INTERVAL) {
                last_commit = now;
                db_commit();
            }

            // SEND POSITION TO SERVER //
            if (now - last_update > 0.1) {
                last_update = now;
                client_position(s->x, s->y, s->z, s->rx, s->ry);
            }

            // PREPARE TO RENDER //
            update_sky_tint();
            g->observe1 = g->observe1 % g->player_count;
            g->observe2 = g->observe2 % g->player_count;
            delete_chunks();
            del_buffer(me->buffer);
            me->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
            for (int i = 1; i < g->player_count; i++) {
                interpolate_player(g->players + i);
            }
            Player *player = g->players + g->observe1;

            // RENDER 3-D SCENE //
            glClear(GL_COLOR_BUFFER_BIT);
            glClear(GL_DEPTH_BUFFER_BIT);
            render_sky(&sky_attrib, player, sky_buffer);
            glClear(GL_DEPTH_BUFFER_BIT);
            int face_count = render_chunks(&block_attrib, player);
            render_signs(&text_attrib, player);
            render_sign(&text_attrib, player);
            render_players(&block_attrib, player);
            if (SHOW_WIREFRAME) {
                render_wireframe(&line_attrib, player);
            }

            // RENDER HUD //
            glClear(GL_DEPTH_BUFFER_BIT);
            if (SHOW_CROSSHAIRS) {
                render_crosshairs(&line_attrib);
            }
            char text_buffer[1024];
            float ts = 12 * g->scale;
            float tx = ts / 2;
            float ty = g->height - ts;
            if (SHOW_ITEM) {
                render_item(&block_attrib);
                render_item_count(&text_attrib, ts);
            }

            // RENDER TEXT //
            if (SHOW_INFO_TEXT) {
                int hour = time_of_day() * 24;
                char am_pm = hour < 12 ? 'a' : 'p';
                hour = hour % 12;
                hour = hour ? hour : 12;
                snprintf(
                    text_buffer, 1024,
                    "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d, %d] %d%cm %dfps",
                    chunked(s->x), chunked(s->z), s->x, s->y, s->z,
                    g->player_count, g->chunk_count,
                    face_count * 2, hour, am_pm, fps.fps);
                render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
                ty -= ts * 2;
            }
            if (SHOW_CHAT_TEXT) {
                for (int i = 0; i < MAX_MESSAGES; i++) {
                    int index = (g->message_index + i) % MAX_MESSAGES;
                    if (strlen(g->messages[index])) {
                        render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts,
                            g->messages[index]);
                        ty -= ts * 2;
                    }
                }
            }
            if (g->typing) {
                snprintf(text_buffer, 1024, "> %s", g->typing_buffer);
                render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
                ty -= ts * 2;
            }
            if (SHOW_PLAYER_NAMES) {
                if (player != me) {
                    render_text(&text_attrib, ALIGN_CENTER,
                        g->width / 2, ts, ts, player->name);
                }
                Player *other = player_crosshair(player);
                if (other) {
                    render_text(&text_attrib, ALIGN_CENTER,
                        g->width / 2, g->height / 2 - ts - 24, ts,
                        other->name);
                }
            }

            // RENDER PICTURE IN PICTURE //
            if (g->observe2) {
                player = g->players + g->observe2;

                int pw = 256 * g->scale;
                int ph = 256 * g->scale;
                int offset = 32 * g->scale;
                int pad = 3 * g->scale;
                int sw = pw + pad * 2;
                int sh = ph + pad * 2;

                glEnable(GL_SCISSOR_TEST);
                glScissor(g->width - sw - offset + pad, offset - pad, sw, sh);
                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_SCISSOR_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);
                glViewport(g->width - pw - offset, offset, pw, ph);

                g->width = pw;
                g->height = ph;
                g->ortho = 0;
                g->fov = 65;

                render_sky(&sky_attrib, player, sky_buffer);
                glClear(GL_DEPTH_BUFFER_BIT);
                render_chunks(&block_attrib, player);
                render_signs(&text_attrib, player);
                render_players(&block_attrib, player);
                glClear(GL_DEPTH_BUFFER_BIT);
                if (SHOW_PLAYER_NAMES) {
                    render_text(&text_attrib, ALIGN_CENTER,
                        pw / 2, ts, ts, player->name);
                }
            }

            // SWAP AND POLL //
            glfwSwapBuffers(g->window);
            glfwPollEvents();
            if (glfwWindowShouldClose(g->window)) {
                running = 0;
                break;
            }
            if (g->mode_changed) {
                g->mode_changed = 0;
                break;
            }
        }

        // SHUTDOWN //
        db_save_state(s->x, s->y, s->z, s->rx, s->ry);
        db_close();
        db_disable();
        client_stop();
        client_disable();
        del_buffer(sky_buffer);
        delete_all_chunks();
        delete_all_players();
    }

    glfwTerminate();
    curl_global_cleanup();
    return 0;
}
