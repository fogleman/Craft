#ifndef __APPLE_CC__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <unistd.h>
#include "client.h"
#include "config.h"
#include "cube.h"
#include "db.h"
#include "map.h"
#include "matrix.h"
#include "noise.h"
#include "util.h"
#include "world.h"

#define MAX_CHUNKS 1024
#define MAX_PLAYERS 128
#define CREATE_CHUNK_RADIUS 6
#define RENDER_CHUNK_RADIUS 6
#define DELETE_CHUNK_RADIUS 12
#define MAX_RECV_LENGTH 1024
#define MAX_TEXT_LENGTH 256
#define MAX_NAME_LENGTH 32
#define LEFT 0
#define CENTER 1
#define RIGHT 2

typedef struct {
    Map map;
    int p;
    int q;
    int faces;
    int dirty;
    GLuint buffer;
} Chunk;

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
    GLuint sampler2;
    GLuint camera;
    GLuint timer;
} Attrib;

static GLFWwindow *window;
static int width = 0;
static int height = 0;
static Chunk chunks[MAX_CHUNKS];
static int chunk_count = 0;
static Player players[MAX_PLAYERS];
static int player_count = 0;
static int exclusive = 1;
static int drop = 0;
static int left_click = 0;
static int right_click = 0;
static int middle_click = 0;
static int observe1 = 0;
static int observe2 = 0;
static int flying = 0;
static int ortho = 0;
static float fov = 65;
static int typing = 0;
static char typing_buffer[MAX_TEXT_LENGTH] = {0};
static Inventory inventory;
static int inventory_screen = 0;
static int inventory_toggle = 0;

int is_plant(int w) {
    return w > 16;
}

int is_obstacle(int w) {
    w = ABS(w);
    return w > 0 && w < 16;
}

int is_transparent(int w) {
    w = ABS(w);
    return w == 0 || w == 10 || w == 15 || is_plant(w);
}

int is_destructable(int w) {
    return w > 0 && w != 16;
}

int is_selectable(int w) {
    return w > 0 && w <= 15;
}

int chunked(float x) {
    return floorf(roundf(x) / CHUNK_SIZE);
}

float time_of_day() {
    float t;
    t = glfwGetTime();
    t = t + DAY_LENGTH / 3.0;
    t = t / DAY_LENGTH;
    t = t - (int)t;
    return t;
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
            y = 0;
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
    int x = width / 2;
    int y = height / 2;
    int p = 10;
    float data[] = {
        x, y - p, x, y + p,
        x - p, y, x + p, y
    };
    return gen_buffer(sizeof(data), data);
}

GLuint gen_wireframe_buffer(float x, float y, float z, float n) {
    float data[144];
    make_cube_wireframe(data, x, y, z, n);
    return gen_buffer(sizeof(data), data);
}

GLuint gen_sky_buffer() {
    float data[12288];
    make_sphere(data, 1, 3);
    return gen_buffer(sizeof(data), data);
}

GLuint gen_cube_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(8, 6);
    make_cube(data, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
    return gen_faces(8, 6, data);
}

GLuint gen_plant_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(8, 4);
    float rotation = simplex3(x, y, z, 4, 0.5, 2) * 360;
    make_plant(data, x, y, z, n, w, rotation);
    return gen_faces(8, 4, data);
}

GLuint gen_player_buffer(float x, float y, float z, float rx, float ry) {
    GLfloat *data = malloc_faces(8, 6);
    make_player(data, x, y, z, rx, ry);
    return gen_faces(8, 6, data);
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

GLuint gen_inventory_buffers(float x, float y, float n, int sel) {
    int length = INVENTORY_SLOTS;
    GLfloat *data = malloc_faces(4, length);
    x -= n * (length - 1) / 2;
    for (int i = 0; i < length; i ++) {
        make_inventory(
                       data + i * 24,
                       x, y, n / 2, n / 2, sel == i ? 1 : 0);
        x += n;
    }
    return gen_faces(4, length, data);
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

void draw_inventory(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(attrib, buffer, length * 6);
    glDisable(GL_BLEND);
}

void draw_chunk(Attrib *attrib, Chunk *chunk) {
    draw_triangles_3d(attrib, chunk->buffer, chunk->faces * 6);
}

void draw_item(Attrib *attrib, GLuint buffer, int count) {
    draw_triangles_3d(attrib, buffer, count);
}

void draw_text(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(attrib, buffer, length * 6);
    glDisable(GL_BLEND);
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

void print(
           Attrib *attrib, int justify,
           float x, float y, float n, char *text)
{
    int length = strlen(text);
    x -= n * justify * (length - 1) / 2;
    GLuint buffer = gen_text_buffer(x, y, n, text);
    draw_text(attrib, buffer, length);
    del_buffer(buffer);
}

Player *find_player(int id) {
    for (int i = 0; i < player_count; i++) {
        Player *player = players + i;
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
    int count = player_count;
    del_buffer(player->buffer);
    Player *other = players + (--count);
    memcpy(player, other, sizeof(Player));
    player_count = count;
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
    for (int i = 0; i < player_count; i++) {
        Player *other = players + i;
        if (other == player) {
            continue;
        }
        float p = player_crosshair_distance(player, other);
        float d = player_player_distance(player, other);
        if (p / d < threshold) {
            if (best == 0 || d < best) {
                best = d;
                result = other;
            }
        }
    }
    return result;
}

Chunk *find_chunk(int p, int q) {
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
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

int chunk_visible(Chunk *chunk, float *matrix) {
    for (int dp = 0; dp <= 1; dp++) {
        for (int dq = 0; dq <= 1; dq++) {
            for (int y = 0; y < 128; y += 16) {
                float vec[4] = {
                    (chunk->p + dp) * CHUNK_SIZE - dp,
                    y,
                    (chunk->q + dq) * CHUNK_SIZE - dq,
                    1};
                mat_vec_multiply(vec, matrix, vec);
                if (vec[3] >= 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
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
        MAP_FOR_EACH(map, e) {
            if (is_obstacle(e->w) && e->x == nx && e->z == nz) {
                result = MAX(result, e->y);
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
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
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

void exposed_faces(
                   Map *map, int x, int y, int z,
                   int *f1, int *f2, int *f3, int *f4, int *f5, int *f6)
{
    *f1 = is_transparent(map_get(map, x - 1, y, z));
    *f2 = is_transparent(map_get(map, x + 1, y, z));
    *f3 = is_transparent(map_get(map, x, y + 1, z));
    *f4 = is_transparent(map_get(map, x, y - 1, z)) && (y > 0);
    *f5 = is_transparent(map_get(map, x, y, z - 1));
    *f6 = is_transparent(map_get(map, x, y, z + 1));
}

void gen_chunk_buffer(Chunk *chunk) {
    Map *map = &chunk->map;
    
    int faces = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        faces += total;
    } END_MAP_FOR_EACH;
    
    GLfloat *data = malloc_faces(8, faces);
    int offset = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        if (total == 0) {
            continue;
        }
        if (is_plant(e->w)) {
            float rotation = simplex3(e->x, e->y, e->z, 4, 0.5, 2) * 360;
            make_plant(
                       data + offset,
                       e->x, e->y, e->z, 0.5, e->w, rotation);
        }
        else {
            make_cube(
                      data + offset,
                      f1, f2, f3, f4, f5, f6,
                      e->x, e->y, e->z, 0.5, e->w);
        }
        offset += total * 48;
    } END_MAP_FOR_EACH;
    
    del_buffer(chunk->buffer);
    chunk->buffer = gen_faces(8, faces, data);
    chunk->faces = faces;
    chunk->dirty = 0;
}

void create_chunk(Chunk *chunk, int p, int q) {
    chunk->p = p;
    chunk->q = q;
    chunk->faces = 0;
    chunk->dirty = 1;
    chunk->buffer = 0;
    Map *map = &chunk->map;
    map_alloc(map);
    create_world(map, p, q);
    db_load_map(map, p, q);
    gen_chunk_buffer(chunk);
    int key = db_get_key(p, q);
    client_chunk(p, q, key);
}

void delete_chunks() {
    int count = chunk_count;
    State *s1 = &players->state;
    State *s2 = &(players + observe1)->state;
    State *s3 = &(players + observe2)->state;
    State *states[3] = {s1, s2, s3};
    for (int i = 0; i < count; i++) {
        Chunk *chunk = chunks + i;
        int delete = 1;
        for (int j = 0; j < 3; j++) {
            State *s = states[j];
            int p = chunked(s->x);
            int q = chunked(s->z);
            if (chunk_distance(chunk, p, q) < DELETE_CHUNK_RADIUS) {
                delete = 0;
                break;
            }
        }
        if (delete) {
            map_free(&chunk->map);
            del_buffer(chunk->buffer);
            Chunk *other = chunks + (--count);
            memcpy(chunk, other, sizeof(Chunk));
        }
    }
    chunk_count = count;
}

void ensure_chunks(float x, float y, float z, int force) {
    int count = chunk_count;
    int p = chunked(x);
    int q = chunked(z);
    int generated = 0;
    int rings = force ? 1 : CREATE_CHUNK_RADIUS;
    for (int ring = 0; ring <= rings; ring++) {
        for (int dp = -ring; dp <= ring; dp++) {
            for (int dq = -ring; dq <= ring; dq++) {
                if (ring != MAX(ABS(dp), ABS(dq))) {
                    continue;
                }
                if (!force && generated && ring > 1) {
                    continue;
                }
                int a = p + dp;
                int b = q + dq;
                Chunk *chunk = find_chunk(a, b);
                if (chunk) {
                    if (chunk->dirty) {
                        gen_chunk_buffer(chunk);
                        generated++;
                    }
                }
                else {
                    if (count < MAX_CHUNKS) {
                        create_chunk(chunks + count, a, b);
                        generated++;
                        count++;
                    }
                }
            }
        }
    }
    chunk_count = count;
}

void _set_block(int p, int q, int x, int y, int z, int w) {
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        Map *map = &chunk->map;
        if (map_get(map, x, y, z) != w) {
            map_set(map, x, y, z, w);
            chunk->dirty = 1;
        }
    }
    db_insert_block(p, q, x, y, z, w);
}

void set_block(int x, int y, int z, int w) {
    int p = chunked(x);
    int q = chunked(z);
    _set_block(p, q, x, y, z, w);
    if (chunked(x - 1) != p) {
        _set_block(p - 1, q, x, y, z, -w);
    }
    if (chunked(x + 1) != p) {
        _set_block(p + 1, q, x, y, z, -w);
    }
    if (chunked(z - 1) != q) {
        _set_block(p, q - 1, x, y, z, -w);
    }
    if (chunked(z + 1) != q) {
        _set_block(p, q + 1, x, y, z, -w);
    }
    client_block(x, y, z, w, inventory.selected);
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

int render_chunks(Attrib *attrib, Player *player) {
    int result = 0;
    State *s = &player->state;
    ensure_chunks(s->x, s->y, s->z, 0);
    int p = chunked(s->x);
    int q = chunked(s->z);
    float matrix[16];
    set_matrix_3d(
                  matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
    glUniform1i(attrib->sampler, 0);
    glUniform1i(attrib->sampler2, 2);
    glUniform1f(attrib->timer, time_of_day());
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
            continue;
        }
        if (s->y < 100 && !chunk_visible(chunk, matrix)) {
            continue;
        }
        draw_chunk(attrib, chunk);
        result += chunk->faces;
    }
    return result;
}

void render_players(Attrib *attrib, Player *player) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
                  matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, time_of_day());
    for (int i = 0; i < player_count; i++) {
        Player *other = players + i;
        if (other != player) {
            draw_player(attrib, other);
        }
    }
}

void render_sky(Attrib *attrib, Player *player, GLuint buffer) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, width, height, 0, 0, 0, s->rx, s->ry, fov, 0);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 3);
    glUniform1f(attrib->timer, time_of_day());
    draw_triangles_3d(attrib, buffer, 512 * 3);
}

void render_wireframe(Attrib *attrib, Player *player) {
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
                  matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (is_obstacle(hw)) {
        glUseProgram(attrib->program);
        glLineWidth(1);
        glEnable(GL_COLOR_LOGIC_OP);
        glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
        GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.53);
        draw_lines(attrib, wireframe_buffer, 3, 48);
        del_buffer(wireframe_buffer);
        glDisable(GL_COLOR_LOGIC_OP);
    }
}

void render_crosshairs(Attrib *attrib) {
    float matrix[16];
    set_matrix_2d(matrix, width, height);
    glUseProgram(attrib->program);
    glLineWidth(4);
    glEnable(GL_COLOR_LOGIC_OP);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    GLuint crosshair_buffer = gen_crosshair_buffer();
    draw_lines(attrib, crosshair_buffer, 2, 4);
    del_buffer(crosshair_buffer);
    glDisable(GL_COLOR_LOGIC_OP);
}

void render_text(
    Attrib *attrib, int justify, float x, float y, float n, char *text)
{
    float matrix[16];
    set_matrix_2d(matrix, width, height);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1);
    int length = strlen(text);
    x -= n * justify * (length - 1) / 2;
    GLuint buffer = gen_text_buffer(x, y, n, text);
    draw_text(attrib, buffer, length);
    del_buffer(buffer);
}

void render_inventory_bar(Attrib *attrib, float x, float y, float n, int sel) {
    float matrix[16];
    set_matrix_2d(matrix, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 2);
    
    GLuint buffer = gen_inventory_buffers(x, y, n, sel);
    
    draw_inventory(attrib, buffer, INVENTORY_SLOTS);
    del_buffer(buffer);
}

void render_inventory_item(Attrib *attrib, Item item, float x, float y, float size) {
    glUseProgram(attrib->program);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, M_PI_2);

    float matrix[16];
    GLuint buffer;

    set_matrix_item(matrix, width, height, size, x, y);
    
    // render selected item
    if (is_plant(item.w)) {
        glDeleteBuffers(1, &buffer);
        buffer = gen_plant_buffer(0, 0, 0, 0.5, item.w);
    }
    else {
        buffer = gen_cube_buffer(0, 0, 0, 0.5, item.w);
    }
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    
    if (is_plant(item.w)) {
        draw_plant(attrib, buffer);
        del_buffer(buffer);
    }
    else {
        draw_cube(attrib, buffer);
        del_buffer(buffer);
    }
}

void render_inventory_items(Attrib *attrib, float x, float y, float size, int row) {
    for (int item = 0; item < INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item + (row * INVENTORY_SLOTS)];
        
        if (block.w == 0)
            continue;
        
        if (block.count == 0)
            continue;

        /* 1  ...  0  ... -1 */
        /* 0 1 2 3 4 5 6 7 8 */
        /* 1 ...  0  ...-1 */
        /* 0 1 2 3 4 5 6 7 */
        float slotoff = ((float)item - (float)(INVENTORY_SLOTS - 1) / 2) * 1.5;
        float xpos = x + slotoff * size;
        
        render_inventory_item(attrib, block, xpos, y, size * 0.75);
    }
}

void render_inventory_text(Attrib *attrib, Item item, float x, float y, float n) {
    float matrix[16];
    // render text
    set_matrix_2d(matrix, width, height);
    
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1);
    
    char text_buffer[16];
    float ts = INVENTORY_FONT_SIZE;
    if (item.count == INVENTORY_UNLIMITED)
        snprintf(text_buffer, 16, CREATIVE_MODE ? "" : "inf");
    else
        snprintf(text_buffer, 16, "%d", item.count);
    x += ts * (2.5 - strlen(text_buffer));
    print(attrib, LEFT,
          x, y, ts, text_buffer);
}

void render_inventory_texts(Attrib *attrib, float x, float y, float n, int row) {
    for (int item = 0; item < INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item + (row * INVENTORY_SLOTS)];
        
        if (block.w == 0)
            continue;
        if (block.count <= 1)
            continue;
        
        float sep = INVENTORY_ITEM_SIZE * 1.5;
        float tx = width / 2 + sep * (item - (((float)INVENTORY_SLOTS - 1.) / 2.));
        float ty = y == 0 ? sep / 3 : y - sep / 3;
        render_inventory_text(attrib, block, tx, ty, n);
    }
}

void render_inventory(Attrib *window_attrib, Attrib *item_attrib, Attrib *text_attrib,
                          float x, float y, float n, int sel) {
    render_inventory_bar(window_attrib, x, y, n, sel);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_inventory_items(item_attrib, x, y, n / 1.5, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_inventory_texts(text_attrib, x, y, n, 0);
}

void render_inventory_screen(Attrib *window_attrib, Attrib *item_attrib, Attrib *text_attrib,
                      float x, float y, float n, int sel) {
    for (int row = 0; row < INVENTORY_ROWS; row ++) {
        render_inventory_bar(window_attrib, x, y + n*row, n, sel - (row * INVENTORY_SLOTS));
        glClear(GL_DEPTH_BUFFER_BIT);
        render_inventory_items(item_attrib, x, y + n*row, n / 1.5, row);
        glClear(GL_DEPTH_BUFFER_BIT);
        render_inventory_texts(text_attrib, x, y + n*row, n, row);
    }
}

void render_inventory_held(Attrib *item_attrib, Attrib *text_attrib,
                           float x, float y, float n) {
    render_inventory_item(item_attrib, inventory.holding, x, height - y, (n / 1.5) * 0.75);
    glClear(GL_DEPTH_BUFFER_BIT);

    if (inventory.holding.count > 1) {
        float sep = INVENTORY_ITEM_SIZE * 1.5;
        render_inventory_text(text_attrib, inventory.holding, x, (height - y) - sep / 3, n);
    }
}

int mouse_to_inventory(int width, int height, float x, float y, float n) {
    /* .. 0 .. 1 .. 2 .. 3 .. 4 .. 5 .. 6 .. 7 .. 8 .. */
    /* |---------------------------------------------| */
    int xcell = round((INVENTORY_SLOTS - 1) / 2. + ((x - width / 2.) / n));
    int ycell = 0.5 - (y - height / 2.) / n;

    if (xcell < 0 || ycell < 0 || xcell >= INVENTORY_SLOTS || ycell >= INVENTORY_ROWS)
        return -1;
    
    return xcell + (ycell * INVENTORY_SLOTS);
}

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        return;
    }
    if (!inventory_screen) {
        if (key == GLFW_KEY_BACKSPACE) {
            if (typing) {
                int n = strlen(typing_buffer);
                if (n > 0) {
                    typing_buffer[n - 1] = '\0';
                }
            }
        }
    }
    if (action != GLFW_PRESS) {
        return;
    }
    if (!inventory_screen) {
        if (key == GLFW_KEY_ESCAPE) {
            if (typing) {
                typing = 0;
            }
            else if (exclusive) {
                exclusive = 0;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        if (key == GLFW_KEY_ENTER) {
            if (typing) {
                typing = 0;
                client_talk(typing_buffer);
            }
            else {
                if (mods & GLFW_MOD_SUPER) {
                    right_click = 1;
                }
                else {
                    left_click = 1;
                }
            }
        }
    }
    if (!typing) {
        if (!inventory_screen) {
            if (key == CRAFT_KEY_FLY) {
                flying = !flying;
            }
            if (key >= '1' && key <= '9') {
                inventory.selected = key - '1';
            }
            if (key == CRAFT_KEY_BLOCK_TYPE) {
                inventory.selected = (inventory.selected + 1) % INVENTORY_SLOTS;
            }
            if (key == CRAFT_KEY_DROP) {
                drop = 1;
            }
            if (key == CRAFT_KEY_OBSERVE) {
                observe1 = (observe1 + 1) % player_count;
            }
            if (key == CRAFT_KEY_OBSERVE_INSET) {
                observe2 = (observe2 + 1) % player_count;
            }
        }
        if (key == CRAFT_KEY_INVENTORY) {
            inventory_toggle = 1;
        }
    }
}

void on_char(GLFWwindow *window, unsigned int u) {
    if (typing) {
        if (u >= 32 && u < 128) {
            char c = (char)u;
            int n = strlen(typing_buffer);
            if (n < MAX_TEXT_LENGTH - 1) {
                typing_buffer[n] = c;
                typing_buffer[n + 1] = '\0';
            }
        }
    }
    else if (!inventory_screen) {
        if (u == CRAFT_KEY_CHAT) {
            typing = 1;
            typing_buffer[0] = '\0';
        }
        if (u == CRAFT_KEY_COMMAND) {
            typing = 1;
            typing_buffer[0] = '/';
            typing_buffer[1] = '\0';
        }
    }
}

void on_scroll(GLFWwindow *window, double xdelta, double ydelta) {
    static double ypos = 0;
    ypos += ydelta;
    if (ypos < -SCROLL_THRESHOLD) {
        inventory.selected++;
        if (inventory.selected >= INVENTORY_SLOTS) {
            inventory.selected = 0;
        }
        ypos = 0;
    }
    if (ypos > SCROLL_THRESHOLD) {
        inventory.selected--;
        if (inventory.selected < 0) {
            inventory.selected = INVENTORY_SLOTS - 1;
        }
        ypos = 0;
    }
}

void on_mouse_button(GLFWwindow *window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (exclusive) {
            if (mods & GLFW_MOD_SUPER) {
                right_click = 1;
            }
            else {
                left_click = 1;
            }
        }
        else {
            if (inventory_screen) {
                double mx, my;
                glfwGetCursorPos(window, &mx, &my);
                
                int sel = mouse_to_inventory(width, height, mx, my, INVENTORY_ITEM_SIZE * 1.5);
                if (inventory.holding.count == 0) {
                    if (sel != -1) {
                        //Pick up
                        inventory.holding.count = inventory.items[sel].count;
                        inventory.holding.w = inventory.items[sel].w;
                        inventory.items[sel].count = 0;
                        inventory.items[sel].w = 0;
                    }
                } else {
                    if (sel == -1) {
                        //Throw
                        inventory.holding.count = 0;
                        inventory.holding.w = 0;
                    } else {
                        //Place
                        if (inventory.items[sel].w == 0) {
                            //Into empty
                            inventory.items[sel].count = inventory.holding.count;
                            inventory.items[sel].w = inventory.holding.w;
                            inventory.holding.count = 0;
                            inventory.holding.w = 0;
                        } else {
                            if (inventory.items[sel].w == inventory.holding.w && inventory.holding.count != INVENTORY_UNLIMITED) {
                                //Into same type
                                if (inventory.holding.count + inventory.items[sel].count <= 64) {
                                    //Append all
                                    inventory.items[sel].count += inventory.holding.count;
                                    inventory.holding.count = 0;
                                    inventory.holding.w = 0;
                                } else {
                                    //Append with leftover
                                    inventory.holding.count -= 64 - inventory.items[sel].count;
                                    inventory.items[sel].count = 64;
                                }
                            } else {
                                //Into diff type
                                Item cache;
                                cache.count = inventory.holding.count;
                                cache.w = inventory.holding.w;
                                
                                inventory.holding = inventory.items[sel];
                                inventory.items[sel] = cache;
                            }
                    
                        }
                    }
                }
            } else {
                exclusive = 1;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (exclusive) {
            right_click = 1;
        } else if (inventory_screen) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            
            int sel = mouse_to_inventory(width, height, mx, my, INVENTORY_ITEM_SIZE * 1.5);
            if (inventory.items[sel].count != INVENTORY_UNLIMITED) {
                if (inventory.holding.count == 0) {
                    if (sel != -1) {
                        //Pick up half
                        int pickup = ceil(inventory.items[sel].count / 2.);
                        
                        inventory.holding.count = pickup;
                        if (inventory.holding.count > 0)
                            inventory.holding.w = inventory.items[sel].w;
                        
                        inventory.items[sel].count -= pickup;
                        if (inventory.items[sel].count == 0)
                            inventory.items[sel].w = 0;
                    }
                } else {
                    if (sel == -1) {
                        //Throw one
                        inventory.holding.count --;
                        if (inventory.holding.count == 0)
                            inventory.holding.w = 0;
                    } else {
                        //Place one
                        if (inventory.items[sel].w == 0) {
                            //Into empty
                            inventory.items[sel].count = 1;
                            inventory.items[sel].w = inventory.holding.w;

                            inventory.holding.count --;
                            if (inventory.holding.count == 0)
                                inventory.holding.w = 0;
                        } else {
                            if (inventory.items[sel].w == inventory.holding.w) {
                                //Into same type
                                if (inventory.items[sel].count < 64) {
                                    //Append with one
                                    inventory.items[sel].count ++;

                                    inventory.holding.count --;
                                    if (inventory.holding.count == 0)
                                        inventory.holding.w = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (exclusive) {
            middle_click = 1;
        } if (inventory_screen) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            
            //TODO: something
        }
    }
}

Item get_current_item() {
    return inventory.items[inventory.selected];
}

int get_current_block() {
    return get_current_item().w;
}

int get_current_count() {
    return get_current_item().count;
}

int find_matching_inventory_slot(int w) {
    //Try for same type
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++)
        if (inventory.items[item].w == w)
            return item;
    return -1;
}

int find_usable_inventory_slot(int w) {
    //Try for infinite
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++)
        if (inventory.items[item].w == w && inventory.items[item].count == INVENTORY_UNLIMITED)
            return -1; //Don't let them pick it up
    //Try for same type
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++)
        if (inventory.items[item].w == w && inventory.items[item].count < MAX_SLOT_SIZE)
            return item;
    //Try for empty
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++)
        if (inventory.items[item].w == 0)
            return item;
    return -1;
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
    window = glfwCreateWindow(
                              window_width, window_height, "Craft", monitor, NULL);
}

int main(int argc, char **argv) {
    // Set dir
    char *base = dirname(argv[0]);
    
    if (chdir(base)) {
        return 1;
    }
    setenv("PWD", base, 1);
    
    srand(time(NULL));
    rand();
    if (argc == 2 || argc == 3) {
        char *hostname = argv[1];
        int port = DEFAULT_PORT;
        if (argc == 3) {
            port = atoi(argv[2]);
        }
        if (USE_CACHE) {
            char path[1024];
            snprintf(path, 1024, "cache.%s.%d.db", hostname, port);
            db_enable();
            if (db_init(path)) {
                return -1;
            }
        }
        client_enable();
        client_connect(hostname, port);
        client_start();
    }
    else {
        db_enable();
        if (db_init(DB_PATH)) {
            return -1;
        }
    }
    if (!glfwInit()) {
        return -1;
    }
    create_window();
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(VSYNC);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, on_key);
    glfwSetCharCallback(window, on_char);
    glfwSetMouseButtonCallback(window, on_mouse_button);
    glfwSetScrollCallback(window, on_scroll);
    
#ifndef __APPLE__
    if (glewInit() != GLEW_OK) {
        return -1;
    }
#endif
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glLogicOp(GL_INVERT);
    glClearColor(0.53, 0.81, 0.92, 1.00);
    
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("texture.png");
    
    GLuint font;
    glGenTextures(1, &font);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    load_png_texture("font.png");
    
    GLuint inventory_texture;
    glGenTextures(1, &inventory_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, inventory_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    load_png_texture("inventory.png");
    
    GLuint sky;
    glGenTextures(1, &sky);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sky);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    load_png_texture("sky.png");

    Attrib block_attrib = {0};
    Attrib line_attrib = {0};
    Attrib text_attrib = {0};
    Attrib inventory_attrib = {0};
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
    block_attrib.sampler2 = glGetUniformLocation(program, "sampler2");
    block_attrib.camera = glGetUniformLocation(program, "camera");
    block_attrib.timer = glGetUniformLocation(program, "timer");
    
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
    
    program = load_program(
                           "shaders/inventory_vertex.glsl", "shaders/inventory_fragment.glsl");
    inventory_attrib.program = program;
    inventory_attrib.position = glGetAttribLocation(program, "position");
    inventory_attrib.uv = glGetAttribLocation(program, "uv");
    inventory_attrib.matrix = glGetUniformLocation(program, "matrix");
    inventory_attrib.sampler = glGetUniformLocation(program, "sampler");
    
    program = load_program(
                           "shaders/sky_vertex.glsl", "shaders/sky_fragment.glsl");
    sky_attrib.program = program;
    sky_attrib.position = glGetAttribLocation(program, "position");
    sky_attrib.normal = glGetAttribLocation(program, "normal");
    sky_attrib.uv = glGetAttribLocation(program, "uv");
    sky_attrib.matrix = glGetUniformLocation(program, "matrix");
    sky_attrib.sampler = glGetUniformLocation(program, "sampler");
    sky_attrib.timer = glGetUniformLocation(program, "timer");

    FPS fps = {0, 0, 0};
    int message_index = 0;
    char messages[MAX_MESSAGES][MAX_TEXT_LENGTH] = {0};
    double last_commit = glfwGetTime();
    double last_update = glfwGetTime();
    GLuint sky_buffer = gen_sky_buffer();

    Player *me = players;
    me->id = 0;
    me->name[0] = '\0';
    me->buffer = 0;
    player_count = 1;
    
    float x = (rand_double() - 0.5) * 10000;
    float z = (rand_double() - 0.5) * 10000;
    float y = 0;
    float rx = 0;
    float ry = 0;
    float dy = 0;
    
    double px = 0;
    double py = 0;
    
    inventory.items = calloc(INVENTORY_SLOTS * INVENTORY_ROWS, sizeof(Item));
    
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++) {
        inventory.items[item].count = 0;
        inventory.items[item].w     = 0;
    }
    
    int loaded = db_load_state(&x, &y, &z, &rx, &ry, &inventory);
    ensure_chunks(x, y, z, 1);
    if (!loaded) {
        y = highest_block(x, z) + 2;
    }
    
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++) {
        if (CREATIVE_MODE) {
            if (is_selectable(item + 1)) {
                inventory.items[item].count = INVENTORY_UNLIMITED;
                inventory.items[item].w = item + 1;
            } else {
                inventory.items[item].count = 0;
                inventory.items[item].w = 0;
            }
        } else {
            if (inventory.items[item].count == INVENTORY_UNLIMITED || inventory.items[item].count > 64) {
                inventory.items[item].count = 64;
            }
        }
        if (inventory.items[item].count <= 0) {
            inventory.items[item].count = 0;
            inventory.items[item].w = 0;
        }
    }
    
    glfwGetCursorPos(window, &px, &py);
    double previous = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        update_fps(&fps);
        double now = glfwGetTime();
        double dt = MIN(now - previous, 0.2);
        previous = now;
        
        if (now - last_commit > COMMIT_INTERVAL) {
            last_commit = now;
            db_commit();
        }
        
        // HANDLE MOUSE INPUT //
        if (exclusive && (px || py)) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            float m = 0.0025;
            rx += (mx - px) * m;
            ry -= (my - py) * m;
            if (rx < 0) {
                rx += RADIANS(360);
            }
            if (rx >= RADIANS(360)){
                rx -= RADIANS(360);
            }
            ry = MAX(ry, -RADIANS(90));
            ry = MIN(ry, RADIANS(90));
            px = mx;
            py = my;
        }
        else {
            glfwGetCursorPos(window, &px, &py);
        }
        
        // HANDLE MOVEMENT //
        float vx = 0, vy = 0, vz = 0;
        int sz = 0;
        int sx = 0;
        
        if (inventory_screen) {
            int sel = mouse_to_inventory(width, height, px, py, INVENTORY_ITEM_SIZE * 1.5);
            inventory.highlighted = sel;
        } else {
            if (!typing) {
                float m = dt * 1.0;
                ortho = glfwGetKey(window, CRAFT_KEY_ORTHO);
                fov = glfwGetKey(window, CRAFT_KEY_ZOOM) ? 15 : 65;
                if (glfwGetKey(window, CRAFT_KEY_QUIT)) break;
                if (glfwGetKey(window, CRAFT_KEY_FORWARD)) sz--;
                if (glfwGetKey(window, CRAFT_KEY_BACKWARD)) sz++;
                if (glfwGetKey(window, CRAFT_KEY_LEFT)) sx--;
                if (glfwGetKey(window, CRAFT_KEY_RIGHT)) sx++;
                if (glfwGetKey(window, GLFW_KEY_LEFT)) rx -= m;
                if (glfwGetKey(window, GLFW_KEY_RIGHT)) rx += m;
                if (glfwGetKey(window, GLFW_KEY_UP)) ry += m;
                if (glfwGetKey(window, GLFW_KEY_DOWN)) ry -= m;
            }
            get_motion_vector(flying, sz, sx, rx, ry, &vx, &vy, &vz);
            if (!typing) {
                if (glfwGetKey(window, CRAFT_KEY_JUMP)) {
                    if (flying) {
                        vy = 1;
                    }
                    else if (dy == 0) {
                        dy = 8;
                    }
                }
                if (glfwGetKey(window, CRAFT_KEY_XM)) {
                    vx = -1; vy = 0; vz = 0;
                }
                if (glfwGetKey(window, CRAFT_KEY_XP)) {
                    vx = 1; vy = 0; vz = 0;
                }
                if (glfwGetKey(window, CRAFT_KEY_YM)) {
                    vx = 0; vy = -1; vz = 0;
                }
                if (glfwGetKey(window, CRAFT_KEY_YP)) {
                    vx = 0; vy = 1; vz = 0;
                }
                if (glfwGetKey(window, CRAFT_KEY_ZM)) {
                    vx = 0; vy = 0; vz = -1;
                }
                if (glfwGetKey(window, CRAFT_KEY_ZP)) {
                    vx = 0; vy = 0; vz = 1;
                }
            }
        }
        float speed = flying ? 20 : 5;
        int step = 8;
        float ut = dt / step;
        vx = vx * ut * speed;
        vy = vy * ut * speed;
        vz = vz * ut * speed;
        for (int i = 0; i < step; i++) {
            if (flying) {
                dy = 0;
            }
            else {
                dy -= ut * 25;
                dy = MAX(dy, -250);
            }
            x += vx;
            y += vy + dy * ut;
            z += vz;
            if (collide(2, &x, &y, &z)) {
                dy = 0;
            }
        }
        if (y < 0) {
            y = highest_block(x, z) + 2;
        }
        if (!inventory_screen) {
            // HANDLE CLICKS //
            if (left_click && exclusive) {
                left_click = 0;
                int hx, hy, hz;
                int hw = hit_test(0, x, y, z, rx, ry,
                                  &hx, &hy, &hz);
                if (hy > 0 && hy < 256 && is_destructable(hw)) {
                    if (is_selectable(hw)) {
                        int slot = find_usable_inventory_slot(hw);
                        
                        if (slot != -1) {
                            inventory.items[slot].w = hw;
                            inventory.items[slot].count ++;
                            db_set_slot(hw, slot, inventory.items[slot].count);
                        }
                    }
                    
                    set_block(hx, hy, hz, 0);
                    int above = get_block(hx, hy + 1, hz);
                    if (is_plant(above)) {
                        set_block(hx, hy + 1, hz, 0);
                    }
                }
            }
            if (right_click && exclusive) {
                right_click = 0;
                int hx, hy, hz;
                int hw = hit_test(1, x, y, z, rx, ry,
                                  &hx, &hy, &hz);
                if (hy > 0 && hy < 256 && is_obstacle(hw)) {
                    if (get_current_count() > 0 &&
                        !player_intersects_block(2, x, y, z, hx, hy, hz)) {
                        
                        set_block(hx, hy, hz, get_current_block());
                        
                        if (get_current_count() != INVENTORY_UNLIMITED) {
                            inventory.items[inventory.selected].count --;
                            if (get_current_count() == 0)
                                inventory.items[inventory.selected].w = 0;
                            db_set_slot(get_current_block(), inventory.selected, get_current_count());
                        }
                    }
                }
            }
            if (middle_click && exclusive) {
                middle_click = 0;
                int hx, hy, hz;
                int hw = hit_test(0, x, y, z, rx, ry,
                                  &hx, &hy, &hz);
                if (is_selectable(hw)) {
                    int slot = find_matching_inventory_slot(hw);
                    if (slot != -1) {
                        if (slot < INVENTORY_SLOTS) {
                            //On your bar
                            inventory.selected = slot;
                        } else {
                            //In your inv
                            if (CREATIVE_MODE) {
                                //Replace held item with new item
                                Item cache;
                                cache.w = inventory.items[inventory.selected].w;
                                cache.count = inventory.items[inventory.selected].count;
                                
                                inventory.items[inventory.selected].w = inventory.items[slot].w;
                                inventory.items[inventory.selected].count = inventory.items[slot].count;
                                
                                inventory.items[slot].w = cache.w;
                                inventory.items[slot].count = cache.count;
                            }
                        }
                    }
                }
            }
            
            if (drop) {
                drop = 0;
                int amount = 1;
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
                    amount = 64;
                
                if (inventory.items[inventory.selected].count != INVENTORY_UNLIMITED) {
                    inventory.items[inventory.selected].count -= amount;
                    if (get_current_count() <= 0) {
                        inventory.items[inventory.selected].count = 0;
                        inventory.items[inventory.selected].w = 0;
                    }
                }
            }
        }
        
        if (inventory_toggle) {
            inventory_toggle = 0;
            inventory_screen = !inventory_screen;
            exclusive = !inventory_screen;
            if (exclusive) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        
        // HANDLE DATA FROM SERVER //
        char buffer[MAX_RECV_LENGTH];
        int count = 0;
        while (count < 1024 && client_recv(buffer, MAX_RECV_LENGTH)) {
            count++;
            int pid;
            float ux, uy, uz, urx, ury;
            if (sscanf(buffer, "U,%d,%f,%f,%f,%f,%f",
                       &pid, &ux, &uy, &uz, &urx, &ury) == 6)
            {
                me->id = pid;
                x = ux; y = uy; z = uz; rx = urx; ry = ury;
                ensure_chunks(x, y, z, 1);
            }
            int bp, bq, bx, by, bz, bw;
            if (sscanf(buffer, "B,%d,%d,%d,%d,%d,%d",
                       &bp, &bq, &bx, &by, &bz, &bw) == 6)
            {
                _set_block(bp, bq, bx, by, bz, bw);
                if (player_intersects_block(2, x, y, z, bx, by, bz)) {
                    y = highest_block(x, z) + 2;
                }
            }
            float px, py, pz, prx, pry;
            if (sscanf(buffer, "P,%d,%f,%f,%f,%f,%f",
                       &pid, &px, &py, &pz, &prx, &pry) == 6)
            {
                Player *player = find_player(pid);
                if (!player && player_count < MAX_PLAYERS) {
                    player = players + player_count;
                    player_count++;
                    player->id = pid;
                    player->buffer = 0;
                    snprintf(player->name, MAX_NAME_LENGTH, "player%d", pid);
                    update_player(player, px, py, pz, prx, pry, 1); // twice
                }
                if (player) {
                    update_player(player, px, py, pz, prx, pry, 1);
                }
            }
            if (sscanf(buffer, "D,%d", &pid) == 1) {
                delete_player(pid);
            }
            int kp, kq, key;
            if (sscanf(buffer, "K,%d,%d,%d", &kp, &kq, &key) == 3) {
                db_set_key(kp, kq, key);
            }
            if (buffer[0] == 'T' && buffer[1] == ',') {
                char *text = buffer + 2;
                printf("%s\n", text);
                snprintf(
                         messages[message_index], MAX_TEXT_LENGTH, "%s", text);
                message_index = (message_index + 1) % MAX_MESSAGES;
            }
            int islot, iw, icount;
            if (sscanf(buffer, "I,%d,%d,%d", &islot, &iw, &icount) == 3) {
                inventory.items[islot].w = iw;
                inventory.items[islot].count = icount;
            }
        }
        
        // SEND DATA TO SERVER //
        if (now - last_update > 0.1) {
            last_update = now;
            client_position(x, y, z, rx, ry);
        }
        
        // PREPARE TO RENDER //
        observe1 = observe1 % player_count;
        observe2 = observe2 % player_count;
        delete_chunks();
        update_player(me, x, y, z, rx, ry, 0);
        for (int i = 1; i < player_count; i++) {
            interpolate_player(players + i);
        }
        Player *player = players + observe1;
        
        // RENDER 3-D SCENE //
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        render_sky(&sky_attrib, player, sky_buffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        int face_count = render_chunks(&block_attrib, player);
        render_players(&block_attrib, player);
        render_wireframe(&line_attrib, player);
        
        // RENDER HUD //
        glClear(GL_DEPTH_BUFFER_BIT);
        render_crosshairs(&line_attrib);
        
        // RENDER TEXT //
        char text_buffer[1024];
        float ts = 12;
        float tx = ts / 2;
        float ty = height - ts;
        int hour = time_of_day() * 24;
        char am_pm = hour < 12 ? 'a' : 'p';
        hour = hour % 12;
        hour = hour ? hour : 12;
        snprintf(
            text_buffer, 1024, "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d, %d] %d%cm %dfps",
            chunked(x), chunked(z), x, y, z,
            player_count, chunk_count, face_count * 2, hour, am_pm, fps.fps);
        render_text(&text_attrib, LEFT, tx, ty, ts, text_buffer);
        for (int i = 0; i < MAX_MESSAGES; i++) {
            int index = (message_index + i) % MAX_MESSAGES;
            if (strlen(messages[index])) {
                ty -= ts * 2;
                render_text(&text_attrib, LEFT, tx, ty, ts, messages[index]);
            }
        }
        if (typing) {
            ty -= ts * 2;
            snprintf(text_buffer, 1024, "> %s", typing_buffer);
            render_text(&text_attrib, LEFT, tx, ty, ts, text_buffer);
        }
        if (player != me) {
            render_text(&text_attrib, CENTER, width / 2, ts, ts, player->name);
        }
        Player *other = player_crosshair(player);
        if (other) {
            render_text(&text_attrib, CENTER,
                width / 2, height / 2 - ts - 24, ts, other->name);
        }
        // RENDER INVENTORY //
        
        render_inventory(&inventory_attrib, &block_attrib, &text_attrib, width / 2, INVENTORY_ITEM_SIZE, INVENTORY_ITEM_SIZE * 1.5, inventory.selected);
        
        if (inventory_screen) {
            render_inventory_screen(&inventory_attrib, &block_attrib, &text_attrib, width / 2, height / 2, INVENTORY_ITEM_SIZE * 1.5, inventory.highlighted);
            if (inventory.holding.count > 0) {
                render_inventory_held(&block_attrib, &text_attrib, px, py, INVENTORY_ITEM_SIZE * 1.5);
            }
        }

        // RENDER PICTURE IN PICTURE //
        if (observe2) {
            player = players + observe2;
            int pw = 256;
            int ph = 256;
            int pad = 3;
            int sw = pw + pad * 2;
            int sh = ph + pad * 2;
            
            glEnable(GL_SCISSOR_TEST);
            glScissor(width - sw - 32 + pad, 32 - pad, sw, sh);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glScissor(width - pw - 32, 32, pw, ph);
            glClearColor(0.53, 0.81, 0.92, 1.00);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(width - pw - 32, 32, pw, ph);
            
            width = pw;
            height = ph;
            ortho = 0;
            fov = 65;

            render_sky(&sky_attrib, player, sky_buffer);
            glClear(GL_DEPTH_BUFFER_BIT);
            render_chunks(&block_attrib, player);
            render_players(&block_attrib, player);
            
            glClear(GL_DEPTH_BUFFER_BIT);
            render_text(&text_attrib, CENTER, pw / 2, ts, ts, player->name);
        }
        // swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    db_save_state(x, y, z, rx, ry, inventory);
    db_close();
    glfwTerminate();
    client_stop();
    return 0;
}
