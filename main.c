#ifndef __APPLE_CC__
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
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
#define RECV_BUFFER_SIZE 1024
#define TEXT_BUFFER_SIZE 256
#define LEFT 0
#define CENTER 1
#define RIGHT 2

static GLFWwindow *window;
static configuration config;
static int exclusive = 1;
static int left_click = 0;
static int right_click = 0;
static int middle_click = 0;
static int teleport = 0;
static int flying = 0;
static int block_type = 1;
static int ortho = 0;
static float fov = 65.0;
static int typing = 0;
static char typing_buffer[TEXT_BUFFER_SIZE] = {0};

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
    State state;
    State state1;
    State state2;
    GLuint buffer;
} Player;

typedef struct {
    GLuint position;
    GLuint normal;
    GLuint uv;
} Attrib;

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
    return w > 0 && w <= 14;
}

int chunked(float x) {
    return floorf(roundf(x) / CHUNK_SIZE);
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

GLuint gen_crosshair_buffer(int width, int height) {
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

Player *find_player(Player *players, int player_count, int id) {
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
        s->x = x; s->y = y + 0.1; s->z = z; s->rx = rx; s->ry = ry;
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

void delete_player(Player *players, int *player_count, int id) {
    Player *player = find_player(players, *player_count, id);
    if (!player) {
        return;
    }
    int count = *player_count;
    del_buffer(player->buffer);
    Player *other = players + (--count);
    memcpy(player, other, sizeof(Player));
    *player_count = count;
}

Chunk *find_chunk(Chunk *chunks, int chunk_count, int p, int q) {
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

int highest_block(Chunk *chunks, int chunk_count, float x, float z) {
    int result = -1;
    int nx = roundf(x);
    int nz = roundf(z);
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(chunks, chunk_count, p, q);
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
    int m = 8;
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
    Chunk *chunks, int chunk_count, int previous,
    float x, float y, float z, float rx, float ry,
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

int collide(
    Chunk *chunks, int chunk_count,
    int height, float *x, float *y, float *z)
{
    int result = 0;
    int p = chunked(*x);
    int q = chunked(*z);
    Chunk *chunk = find_chunk(chunks, chunk_count, p, q);
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

void ensure_chunks(
    Chunk *chunks, int *chunk_count,
    float x, float y, float z, int force)
{
    int p = chunked(x);
    int q = chunked(z);
    int count = *chunk_count;
    for (int i = 0; i < count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) >= DELETE_CHUNK_RADIUS) {
            map_free(&chunk->map);
            del_buffer(chunk->buffer);
            Chunk *other = chunks + (--count);
            memcpy(chunk, other, sizeof(Chunk));
        }
    }
    int rings = force ? 1 : CREATE_CHUNK_RADIUS;
    int generated = 0;
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
                Chunk *chunk = find_chunk(chunks, count, a, b);
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
    *chunk_count = count;
}

void _set_block(
    Chunk *chunks, int chunk_count,
    int p, int q, int x, int y, int z, int w)
{
    Chunk *chunk = find_chunk(chunks, chunk_count, p, q);
    if (chunk) {
        Map *map = &chunk->map;
        if (map_get(map, x, y, z) != w) {
            map_set(map, x, y, z, w);
            chunk->dirty = 1;
        }
    }
    db_insert_block(p, q, x, y, z, w);
}

void set_block(
    Chunk *chunks, int chunk_count,
    int x, int y, int z, int w)
{
    int p = chunked(x);
    int q = chunked(z);
    _set_block(chunks, chunk_count, p, q, x, y, z, w);
    if (chunked(x - 1) != p) {
        _set_block(chunks, chunk_count, p - 1, q, x, y, z, -w);
    }
    if (chunked(x + 1) != p) {
        _set_block(chunks, chunk_count, p + 1, q, x, y, z, -w);
    }
    if (chunked(z - 1) != q) {
        _set_block(chunks, chunk_count, p, q - 1, x, y, z, -w);
    }
    if (chunked(z + 1) != q) {
        _set_block(chunks, chunk_count, p, q + 1, x, y, z, -w);
    }
    client_block(x, y, z, w);
}

int get_block(
    Chunk *chunks, int chunk_count,
    int x, int y, int z)
{
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(chunks, chunk_count, p, q);
    if (chunk) {
        Map *map = &chunk->map;
        return map_get(map, x, y, z);
    }
    return 0;
}

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
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
    if (key == GLFW_KEY_BACKSPACE) {
        if (typing) {
            int n = strlen(typing_buffer);
            if (n > 0) {
                typing_buffer[n - 1] = '\0';
            }
        }
    }
    if (!typing) {
        if (key == config.fly) {
            flying = !flying;
        }
        if (key == config.teleport) {
            teleport = 1;
        }
        if (key >= '1' && key <= '9') {
            block_type = key - '1' + 1;
        }
        if (key == '0') {
            block_type = 10;
        }
        if (key == config.cycle_block) {
            block_type = block_type % 14 + 1;
        }
    }
}

void on_char(GLFWwindow *window, unsigned int u) {
    if (typing) {
        if (u >= 32 && u < 128) {
            char c = (char)u;
            int n = strlen(typing_buffer);
            if (n < TEXT_BUFFER_SIZE - 1) {
                typing_buffer[n] = c;
                typing_buffer[n + 1] = '\0';
            }
        }
    }
    else {
        if (toupper(u) == config.chat) {
            typing = 1;
            typing_buffer[0] = '\0';
        }
        if (u == config.command) {
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
        block_type++;
        if (block_type > 14) {
            block_type = 1;
        }
        ypos = 0;
    }
    if (ypos > SCROLL_THRESHOLD) {
        block_type--;
        if (block_type < 1) {
            block_type = 14;
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
            exclusive = 1;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (exclusive) {
            right_click = 1;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (exclusive) {
            middle_click = 1;
        }
    }
}

void create_window() {
    int width = config.width;
    int height = config.height;
    GLFWmonitor *monitor = NULL;
    if (config.fullscreen) {
        int mode_count;
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *modes = glfwGetVideoModes(monitor, &mode_count);
        width = modes[mode_count - 1].width;
        height = modes[mode_count - 1].height;
    }
    window = glfwCreateWindow(width, height, "Craft", monitor, NULL);
}

int main(int argc, char **argv) {

    if (configure(&config)) {
        fprintf(stderr, "Fatal: configuration failed.\n");
        return 1;
    }

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
    glfwSwapInterval(config.vsync);
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

    Attrib block_attrib = {0, 0, 0};
    Attrib line_attrib = {0, 0, 0};
    Attrib text_attrib = {0, 0, 0};

    GLuint block_program = load_program(
        "shaders/block_vertex.glsl", "shaders/block_fragment.glsl");
    block_attrib.position = glGetAttribLocation(block_program, "position");
    block_attrib.normal = glGetAttribLocation(block_program, "normal");
    block_attrib.uv = glGetAttribLocation(block_program, "uv");
    GLuint matrix_loc = glGetUniformLocation(block_program, "matrix");
    GLuint sampler_loc = glGetUniformLocation(block_program, "sampler");
    GLuint camera_loc = glGetUniformLocation(block_program, "camera");
    GLuint timer_loc = glGetUniformLocation(block_program, "timer");

    GLuint line_program = load_program(
        "shaders/line_vertex.glsl", "shaders/line_fragment.glsl");
    line_attrib.position = glGetAttribLocation(line_program, "position");
    GLuint line_matrix_loc = glGetUniformLocation(line_program, "matrix");

    GLuint text_program = load_program(
        "shaders/text_vertex.glsl", "shaders/text_fragment.glsl");
    text_attrib.position = glGetAttribLocation(text_program, "position");
    text_attrib.uv = glGetAttribLocation(text_program, "uv");
    GLuint text_matrix_loc = glGetUniformLocation(text_program, "matrix");
    GLuint text_sampler_loc = glGetUniformLocation(text_program, "sampler");

    GLuint item_buffer = 0;
    int previous_block_type = 0;
    char messages[MAX_MESSAGES][TEXT_BUFFER_SIZE] = {0};
    int message_index = 0;
    double last_commit = glfwGetTime();
    double last_update = glfwGetTime();

    Chunk chunks[MAX_CHUNKS];
    int chunk_count = 0;

    Player me;
    me.id = 0;
    me.buffer = 0;

    Player players[MAX_PLAYERS];
    int player_count = 0;

    FPS fps = {0, 0, 0};
    float matrix[16];
    float x = (rand_double() - 0.5) * 10000;
    float z = (rand_double() - 0.5) * 10000;
    float y = 0;
    float dy = 0;
    float rx = 0;
    float ry = 0;
    double px = 0;
    double py = 0;

    int loaded = db_load_state(&x, &y, &z, &rx, &ry);
    ensure_chunks(chunks, &chunk_count, x, y, z, 1);
    if (!loaded) {
        y = highest_block(chunks, chunk_count, x, z) + 2;
    }

    glfwGetCursorPos(window, &px, &py);
    double previous = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        int width, height;
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

        int sz = 0;
        int sx = 0;
        if (!typing) {
            float m = dt * 1.0;
            ortho = glfwGetKey(window, config.ortho_view);
            fov = glfwGetKey(window, config.zoom) ? 15.0 : 65.0;
            if (glfwGetKey(window, config.quit)) break;
            if (glfwGetKey(window, config.forward)) sz--;
            if (glfwGetKey(window, config.backward)) sz++;
            if (glfwGetKey(window, config.strafe_left)) sx--;
            if (glfwGetKey(window, config.strafe_right)) sx++;
            if (glfwGetKey(window, GLFW_KEY_LEFT)) rx -= m;
            if (glfwGetKey(window, GLFW_KEY_RIGHT)) rx += m;
            if (glfwGetKey(window, GLFW_KEY_UP)) ry += m;
            if (glfwGetKey(window, GLFW_KEY_DOWN)) ry -= m;
        }
        float vx, vy, vz;
        get_motion_vector(flying, sz, sx, rx, ry, &vx, &vy, &vz);
        if (!typing) {
            if (glfwGetKey(window, config.jump)) {
                if (flying) {
                    vy = 1;
                }
                else if (dy == 0) {
                    dy = 8;
                }
            }
            if (glfwGetKey(window, config.x_dec)) {
                vx = -1; vy = 0; vz = 0;
            }
            if (glfwGetKey(window, config.x_inc)) {
                vx = 1; vy = 0; vz = 0;
            }
            if (glfwGetKey(window, config.y_dec)) {
                vx = 0; vy = -1; vz = 0;
            }
            if (glfwGetKey(window, config.y_inc)) {
                vx = 0; vy = 1; vz = 0;
            }
            if (glfwGetKey(window, config.z_dec)) {
                vx = 0; vy = 0; vz = -1;
            }
            if (glfwGetKey(window, config.z_inc)) {
                vx = 0; vy = 0; vz = 1;
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
            if (collide(chunks, chunk_count, 2, &x, &y, &z)) {
                dy = 0;
            }
        }
        if (y < 0) {
            y = highest_block(chunks, chunk_count, x, z) + 2;
        }

        if (left_click) {
            left_click = 0;
            int hx, hy, hz;
            int hw = hit_test(chunks, chunk_count, 0, x, y, z, rx, ry,
                &hx, &hy, &hz);
            if (hy > 0 && hy < 256 && is_destructable(hw)) {
                set_block(chunks, chunk_count, hx, hy, hz, 0);
                int above = get_block(chunks, chunk_count, hx, hy + 1, hz);
                if (is_plant(above)) {
                    set_block(chunks, chunk_count, hx, hy + 1, hz, 0);
                }
            }
        }

        if (right_click) {
            right_click = 0;
            int hx, hy, hz;
            int hw = hit_test(chunks, chunk_count, 1, x, y, z, rx, ry,
                &hx, &hy, &hz);
            if (hy > 0 && hy < 256 && is_obstacle(hw)) {
                if (!player_intersects_block(2, x, y, z, hx, hy, hz)) {
                    set_block(chunks, chunk_count, hx, hy, hz, block_type);
                }
            }
        }

        if (middle_click) {
            middle_click = 0;
            int hx, hy, hz;
            int hw = hit_test(chunks, chunk_count, 0, x, y, z, rx, ry,
                &hx, &hy, &hz);
            if (is_selectable(hw)) {
                block_type = hw;
            }
        }

        if (teleport) {
            teleport = 0;
            if (player_count) {
                int index = rand_int(player_count);
                Player *player = players + index;
                State *s = &player->state;
                x = s->x; y = s->y; z = s->z;
                rx = s->rx; ry = s->ry;
                ensure_chunks(chunks, &chunk_count, x, y, z, 1);
            }
        }

        char buffer[RECV_BUFFER_SIZE];
        int count = 0;
        while (count < 1024 && client_recv(buffer, RECV_BUFFER_SIZE)) {
            count++;
            float ux, uy, uz, urx, ury;
            if (sscanf(buffer, "U,%*d,%f,%f,%f,%f,%f",
                &ux, &uy, &uz, &urx, &ury) == 5)
            {
                x = ux; y = uy; z = uz; rx = urx; ry = ury;
                ensure_chunks(chunks, &chunk_count, x, y, z, 1);
            }
            int bp, bq, bx, by, bz, bw;
            if (sscanf(buffer, "B,%d,%d,%d,%d,%d,%d",
                &bp, &bq, &bx, &by, &bz, &bw) == 6)
            {
                _set_block(chunks, chunk_count, bp, bq, bx, by, bz, bw);
                if (player_intersects_block(2, x, y, z, bx, by, bz)) {
                    y = highest_block(chunks, chunk_count, x, z) + 2;
                }
            }
            int pid;
            float px, py, pz, prx, pry;
            if (sscanf(buffer, "P,%d,%f,%f,%f,%f,%f",
                &pid, &px, &py, &pz, &prx, &pry) == 6)
            {
                Player *player = find_player(players, player_count, pid);
                if (!player && player_count < MAX_PLAYERS) {
                    player = players + player_count;
                    player_count++;
                    player->id = pid;
                    player->buffer = 0;
                    update_player(player, px, py, pz, prx, pry, 1); // twice
                }
                if (player) {
                    update_player(player, px, py, pz, prx, pry, 1);
                }
            }
            if (sscanf(buffer, "D,%d", &pid) == 1) {
                delete_player(players, &player_count, pid);
            }
            int kp, kq, key;
            if (sscanf(buffer, "K,%d,%d,%d", &kp, &kq, &key) == 3) {
                db_set_key(kp, kq, key);
            }
            if (buffer[0] == 'T' && buffer[1] == ',') {
                char *text = buffer + 2;
                printf("%s\n", text);
                snprintf(
                    messages[message_index], TEXT_BUFFER_SIZE, "%s", text);
                message_index = (message_index + 1) % MAX_MESSAGES;
            }
        }

        if (now - last_update > 0.1) {
            last_update = now;
            client_position(x, y, z, rx, ry);
        }

        int p = chunked(x);
        int q = chunked(z);
        ensure_chunks(chunks, &chunk_count, x, y, z, 0);
        update_player(&me, x, y, z, rx, ry, 0);

        // RENDER 3-D SCENE //

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        set_matrix_3d(matrix, width, height, x, y, z, rx, ry, fov, ortho);

        // render chunks
        glUseProgram(block_program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform3f(camera_loc, x, y, z);
        glUniform1i(sampler_loc, 0);
        glUniform1f(timer_loc, glfwGetTime());
        for (int i = 0; i < chunk_count; i++) {
            Chunk *chunk = chunks + i;
            if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
                continue;
            }
            if (y < 100 && !chunk_visible(chunk, matrix)) {
                continue;
            }
            draw_chunk(&block_attrib, chunk);
        }

        // render players
        // draw_player(&block_attrib, &me);
        for (int i = 0; i < player_count; i++) {
            Player *player = players + i;
            interpolate_player(player);
            draw_player(&block_attrib, player);
        }

        // render focused block wireframe
        int hx, hy, hz;
        int hw = hit_test(
            chunks, chunk_count, 0, x, y, z, rx, ry, &hx, &hy, &hz);
        if (is_obstacle(hw)) {
            glUseProgram(line_program);
            glLineWidth(1);
            glEnable(GL_COLOR_LOGIC_OP);
            glUniformMatrix4fv(line_matrix_loc, 1, GL_FALSE, matrix);
            GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.53);
            draw_lines(&line_attrib, wireframe_buffer, 3, 48);
            del_buffer(wireframe_buffer);
            glDisable(GL_COLOR_LOGIC_OP);
        }

        // RENDER 2-D HUD PARTS //

        glClear(GL_DEPTH_BUFFER_BIT);
        set_matrix_2d(matrix, width, height);

        // render crosshairs
        glUseProgram(line_program);
        glLineWidth(4);
        glEnable(GL_COLOR_LOGIC_OP);
        glUniformMatrix4fv(line_matrix_loc, 1, GL_FALSE, matrix);
        GLuint crosshair_buffer = gen_crosshair_buffer(width, height);
        draw_lines(&line_attrib, crosshair_buffer, 2, 4);
        del_buffer(crosshair_buffer);
        glDisable(GL_COLOR_LOGIC_OP);

        // render text
        glUseProgram(text_program);
        glUniformMatrix4fv(text_matrix_loc, 1, GL_FALSE, matrix);
        glUniform1i(text_sampler_loc, 1);
        char text_buffer[1024];
        float ts = 12;
        float tx = ts / 2;
        float ty = height - ts;
        snprintf(
            text_buffer, 1024, "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d] %d",
            p, q, x, y, z, player_count, chunk_count, fps.fps);
        print(&text_attrib, LEFT, tx, ty, ts, text_buffer);
        for (int i = 0; i < MAX_MESSAGES; i++) {
            int index = (message_index + i) % MAX_MESSAGES;
            if (strlen(messages[index])) {
                ty -= ts * 2;
                print(&text_attrib, LEFT, tx, ty, ts, messages[index]);
            }
        }
        if (typing) {
            ty -= ts * 2;
            snprintf(text_buffer, 1024, "> %s", typing_buffer);
            print(&text_attrib, LEFT, tx, ty, ts, text_buffer);
        }

        // RENDER 3-D HUD PARTS //

        set_matrix_item(matrix, width, height);

        // render selected item
        if (block_type != previous_block_type) {
            previous_block_type = block_type;
            if (is_plant(block_type)) {
                del_buffer(item_buffer);
                item_buffer = gen_plant_buffer(0, 0, 0, 0.5, block_type);
            }
            else {
                del_buffer(item_buffer);
                item_buffer = gen_cube_buffer(0, 0, 0, 0.5, block_type);
            }
        }
        glUseProgram(block_program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform3f(camera_loc, 0, 0, 5);
        glUniform1i(sampler_loc, 0);
        glUniform1f(timer_loc, glfwGetTime());
        if (is_plant(block_type)) {
            draw_plant(&block_attrib, item_buffer);
        }
        else {
            draw_cube(&block_attrib, item_buffer);
        }

        // swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    db_save_state(x, y, z, rx, ry);
    db_close();
    glfwTerminate();
    client_stop();
    return 0;
}
