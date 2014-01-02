#ifdef _WIN32
    #include <windows.h>
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "client.h"
#include "config.h"
#include "cube.h"
#include "db.h"
#include "item.h"
#include "map.h"
#include "matrix.h"
#include "noise.h"
#include "sign.h"
#include "util.h"
#include "world.h"

#define MAX_CHUNKS 1024
#define MAX_PLAYERS 128
#define MAX_RECV_LENGTH 1024
#define MAX_TEXT_LENGTH 256
#define MAX_NAME_LENGTH 32

#define LEFT 0
#define CENTER 1
#define RIGHT 2

typedef struct {
    Map map;
    SignList signs;
    int p;
    int q;
    int faces;
    int sign_faces;
    int dirty;
    GLuint buffer;
    GLuint sign_buffer;
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
    GLuint camera;
    GLuint timer;
    GLuint extra1;
    GLuint extra2;
    GLuint extra3;
    GLuint extra4;
} Attrib;

static GLFWwindow *window;
static int width = 0;
static int height = 0;
static Chunk chunks[MAX_CHUNKS];
static int chunk_count = 0;
static Player players[MAX_PLAYERS];
static int player_count = 0;
static int exclusive = 1;
static int left_click = 0;
static int right_click = 0;
static int middle_click = 0;
static int observe1 = 0;
static int observe2 = 0;
static int flying = 0;
static int item_index = 0;
static int scale = 1;
static int ortho = 0;
static float fov = 65;
static int typing = 0;
static char typing_buffer[MAX_TEXT_LENGTH] = {0};

int chunked(float x) {
    return floorf(roundf(x) / CHUNK_SIZE);
}

float time_of_day() {
    if (DAY_LENGTH <= 0) {
        return 0.5;
    }
    float t;
    t = glfwGetTime();
    t = t + DAY_LENGTH / 3.0;
    t = t / DAY_LENGTH;
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
        float t = (timer - 0.90) * 100;
        return 1 - 1 / (1 + powf(2, -t));
    }
}

int get_scale_factor() {
    int window_width, window_height;
    int buffer_width, buffer_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
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
    int x = width / 2;
    int y = height / 2;
    int p = 10 * scale;
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
    GLfloat *data = malloc_faces(9, 6);
    float ao[6][4] = {0};
    make_cube(data, ao, 1, 1, 1, 1, 1, 1, x, y, z, n, w);
    return gen_faces(9, 6, data);
}

GLuint gen_plant_buffer(float x, float y, float z, float n, int w) {
    GLfloat *data = malloc_faces(9, 4);
    make_plant(data, x, y, z, n, w, 45);
    return gen_faces(9, 4, data);
}

GLuint gen_player_buffer(float x, float y, float z, float rx, float ry) {
    GLfloat *data = malloc_faces(9, 6);
    make_player(data, x, y, z, rx, ry);
    return gen_faces(9, 6, data);
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
        sizeof(GLfloat) * 9, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 9, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 3, GL_FLOAT, GL_FALSE,
        sizeof(GLfloat) * 9, (GLvoid *)(sizeof(GLfloat) * 6));
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
    glPolygonOffset(-64, -64);
    draw_triangles_3d_text(attrib, chunk->sign_buffer, chunk->sign_faces * 6);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void draw_sign(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-64, -64);
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

int chunk_visible(Chunk *chunk, float planes[6][4]) {
    int x = chunk->p * CHUNK_SIZE - 1;
    int z = chunk->q * CHUNK_SIZE - 1;
    int d = CHUNK_SIZE + 1;
    float points[8][3] = {
        {x + 0, 0, z + 0},
        {x + d, 0, z + 0},
        {x + 0, 0, z + d},
        {x + d, 0, z + d},
        {x + 0, 256, z + 0},
        {x + d, 256, z + 0},
        {x + 0, 256, z + d},
        {x + d, 256, z + d}
    };
    int p = ortho ? 4 : 6;
    for (int i = 0; i < p; i++) {
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

void occlusion(char neighbors[27], float result[6][4]) {
    static int lookup[6][4][3] =
    {
        {
            {0, 1, 3},
            {2, 1, 5},
            {6, 3, 7},
            {8, 5, 7}
        },
        {
            {18, 19, 21},
            {20, 19, 23},
            {24, 21, 25},
            {26, 23, 25}
        },
        {
            {6, 7, 15},
            {8, 7, 17},
            {24, 15, 25},
            {26, 17, 25}
        },
        {
            {0, 1, 9},
            {2, 1, 11},
            {18, 9, 19},
            {20, 11, 19}
        },
        {
            {0, 3, 9},
            {6, 3, 15},
            {18, 9, 21},
            {24, 15, 21}
        },
        {
            {2, 5, 11},
            {8, 5, 17},
            {20, 11, 23},
            {26, 17, 23}
        }
    };
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            int corner = neighbors[lookup[i][j][0]];
            int side1 = neighbors[lookup[i][j][1]];
            int side2 = neighbors[lookup[i][j][2]];
            int value = side1 && side2 ? 3 : corner + side1 + side2;
            result[i][j] = value / 3.0;
        }
    }
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
                    data + count * 30, rx, ry, rz, n / 2, n, face, line[i]);
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

void gen_chunk_buffer(Chunk *chunk) {
    static char blocks[CHUNK_SIZE + 2][258][CHUNK_SIZE + 2];
    static char neighbors[27];
    memset(blocks, 0, sizeof(blocks));
    memset(neighbors, 0, sizeof(neighbors));
    int ox = chunk->p * CHUNK_SIZE - 1;
    int oy = -1;
    int oz = chunk->q * CHUNK_SIZE - 1;

    Map *map = &chunk->map;

    // first pass - populate blocks array
    MAP_FOR_EACH(map, e) {
        int x = e->x - ox;
        int y = e->y - oy;
        int z = e->z - oz;
        // TODO: this should be unnecessary
        if (x < 0 || y < 0 || z < 0) {
            continue;
        }
        if (x >= CHUNK_SIZE + 2 || y >= 258 || z >= CHUNK_SIZE + 2) {
            continue;
        }
        // END TODO
        blocks[x][y][z] = e->w;
    } END_MAP_FOR_EACH;

    // second pass - count exposed faces
    int faces = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int x = e->x - ox;
        int y = e->y - oy;
        int z = e->z - oz;
        int f1 = is_transparent(blocks[x - 1][y][z]);
        int f2 = is_transparent(blocks[x + 1][y][z]);
        int f3 = is_transparent(blocks[x][y + 1][z]);
        int f4 = is_transparent(blocks[x][y - 1][z]) && (e->y > 0);
        int f5 = is_transparent(blocks[x][y][z - 1]);
        int f6 = is_transparent(blocks[x][y][z + 1]);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        faces += total;
    } END_MAP_FOR_EACH;

    // third pass - generate geometry
    GLfloat *data = malloc_faces(9, faces);
    int offset = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int x = e->x - ox;
        int y = e->y - oy;
        int z = e->z - oz;
        int f1 = is_transparent(blocks[x - 1][y][z]);
        int f2 = is_transparent(blocks[x + 1][y][z]);
        int f3 = is_transparent(blocks[x][y + 1][z]);
        int f4 = is_transparent(blocks[x][y - 1][z]) && (e->y > 0);
        int f5 = is_transparent(blocks[x][y][z - 1]);
        int f6 = is_transparent(blocks[x][y][z + 1]);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (is_plant(e->w)) {
            total = total ? 4 : 0;
        }
        if (total == 0) {
            continue;
        }
        if (is_plant(e->w)) {
            float rotation = simplex2(e->x, e->z, 4, 0.5, 2) * 360;
            make_plant(
                data + offset,
                e->x, e->y, e->z, 0.5, e->w, rotation);
        }
        else {
            int index = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dz = -1; dz <= 1; dz++) {
                        int w = blocks[x + dx][y + dy][z + dz];
                        neighbors[index++] = !is_transparent(w);
                    }
                }
            }
            float ao[6][4];
            occlusion(neighbors, ao);
            make_cube(
                data + offset, ao,
                f1, f2, f3, f4, f5, f6,
                e->x, e->y, e->z, 0.5, e->w);
        }
        offset += total * 54;
    } END_MAP_FOR_EACH;

    del_buffer(chunk->buffer);
    chunk->buffer = gen_faces(9, faces, data);
    chunk->faces = faces;

    gen_sign_buffer(chunk);

    chunk->dirty = 0;
}

void create_chunk(Chunk *chunk, int p, int q) {
    chunk->p = p;
    chunk->q = q;
    chunk->faces = 0;
    chunk->sign_faces = 0;
    chunk->dirty = 1;
    chunk->buffer = 0;
    chunk->sign_buffer = 0;
    Map *map = &chunk->map;
    SignList *signs = &chunk->signs;
    map_alloc(map);
    sign_list_alloc(signs, 16);
    create_world(map, p, q);
    db_load_map(map, p, q);
    db_load_signs(signs, p, q);
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
            sign_list_free(&chunk->signs);
            del_buffer(chunk->buffer);
            del_buffer(chunk->sign_buffer);
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

void unset_sign(int x, int y, int z) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        sign_list_remove_all(signs, x, y, z);
        chunk->dirty = 1;
    }
    db_delete_signs(x, y, z);
}

void unset_sign_face(int x, int y, int z, int face) {
    int p = chunked(x);
    int q = chunked(z);
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        sign_list_remove(signs, x, y, z, face);
        chunk->dirty = 1;
    }
    db_delete_sign(x, y, z, face);
}

void _set_sign(int p, int q, int x, int y, int z, int face, const char *text) {
    if (strlen(text) == 0) {
        unset_sign_face(x, y, z, face);
        return;
    }
    Chunk *chunk = find_chunk(p, q);
    if (chunk) {
        SignList *signs = &chunk->signs;
        sign_list_add(signs, x, y, z, face, text);
        chunk->dirty = 1;
    }
    db_insert_sign(p, q, x, y, z, face, text);
}

void set_sign(int x, int y, int z, int face, const char *text) {
    int p = chunked(x);
    int q = chunked(z);
    _set_sign(p, q, x, y, z, face, text);
    client_sign(x, y, z, face, text);
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
    if (w == 0 && chunked(x) == p && chunked(z) == q) {
        unset_sign(x, y, z);
    }
}

void set_block(int x, int y, int z, int w) {
    int p = chunked(x);
    int q = chunked(z);
    _set_block(p, q, x, y, z, w);
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
            _set_block(p + dx, q + dz, x, y, z, -w);
        }
    }
    client_block(x, y, z, w);
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
    float light = get_daylight();
    float matrix[16];
    set_matrix_3d(
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    float planes[6][4];
    frustum_planes(planes, matrix);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
    glUniform1i(attrib->sampler, 0);
    glUniform1i(attrib->extra1, 2);
    glUniform1f(attrib->extra2, light);
    glUniform1f(attrib->extra3, RENDER_CHUNK_RADIUS * CHUNK_SIZE);
    glUniform1i(attrib->extra4, ortho);
    glUniform1f(attrib->timer, time_of_day());
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
            continue;
        }
        if (!chunk_visible(chunk, planes)) {
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
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    float planes[6][4];
    frustum_planes(planes, matrix);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 3);
    glUniform1i(attrib->extra1, 1);
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
            continue;
        }
        if (!chunk_visible(chunk, planes)) {
            continue;
        }
        draw_signs(attrib, chunk);
    }
}

void render_sign(Attrib *attrib, Player *player) {
    if (!typing || typing_buffer[0] != CRAFT_KEY_SIGN) {
        return;
    }
    int x, y, z, face;
    if (!hit_test_face(player, &x, &y, &z, &face)) {
        return;
    }
    State *s = &player->state;
    float matrix[16];
    set_matrix_3d(
        matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 3);
    glUniform1i(attrib->extra1, 1);
    char text[MAX_SIGN_LENGTH];
    strncpy(text, typing_buffer + 1, MAX_SIGN_LENGTH);
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
    glUniform1i(attrib->sampler, 2);
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
    glLineWidth(4 * scale);
    glEnable(GL_COLOR_LOGIC_OP);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    GLuint crosshair_buffer = gen_crosshair_buffer();
    draw_lines(attrib, crosshair_buffer, 2, 4);
    del_buffer(crosshair_buffer);
    glDisable(GL_COLOR_LOGIC_OP);
}

void render_item(Attrib *attrib) {
    float matrix[16];
    set_matrix_item(matrix, width, height, scale);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, time_of_day());
    int w = items[item_index];
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
}

void render_text(
    Attrib *attrib, int justify, float x, float y, float n, char *text)
{
    float matrix[16];
    set_matrix_2d(matrix, width, height);
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

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        return;
    }
    if (key == GLFW_KEY_BACKSPACE) {
        if (typing) {
            int n = strlen(typing_buffer);
            if (n > 0) {
                typing_buffer[n - 1] = '\0';
            }
        }
    }
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
            if (mods & GLFW_MOD_SHIFT) {
                int n = strlen(typing_buffer);
                if (n < MAX_TEXT_LENGTH - 1) {
                    typing_buffer[n] = '\n';
                    typing_buffer[n + 1] = '\0';
                }
            }
            else {
                typing = 0;
                if (typing_buffer[0] == CRAFT_KEY_SIGN) {
                    Player *player = players;
                    int x, y, z, face;
                    if (hit_test_face(player, &x, &y, &z, &face)) {
                        set_sign(x, y, z, face, typing_buffer + 1);
                    }
                }
                else {
                    client_talk(typing_buffer);
                }
            }
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
    if (!typing) {
        if (key == CRAFT_KEY_FLY) {
            flying = !flying;
        }
        if (key >= '1' && key <= '9') {
            item_index = key - '1';
        }
        if (key == '0') {
            item_index = 9;
        }
        if (key == CRAFT_KEY_ITEM_NEXT) {
            item_index = (item_index + 1) % item_count;
        }
        if (key == CRAFT_KEY_ITEM_PREV) {
            item_index--;
            if (item_index < 0) {
                item_index = item_count - 1;
            }
        }
        if (key == CRAFT_KEY_OBSERVE) {
            observe1 = (observe1 + 1) % player_count;
        }
        if (key == CRAFT_KEY_OBSERVE_INSET) {
            observe2 = (observe2 + 1) % player_count;
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
    else {
        if (u == CRAFT_KEY_CHAT) {
            typing = 1;
            typing_buffer[0] = '\0';
        }
        if (u == CRAFT_KEY_COMMAND) {
            typing = 1;
            typing_buffer[0] = '/';
            typing_buffer[1] = '\0';
        }
        if (u == CRAFT_KEY_SIGN) {
            typing = 1;
            typing_buffer[0] = CRAFT_KEY_SIGN;
            typing_buffer[1] = '\0';
        }
    }
}

void on_scroll(GLFWwindow *window, double xdelta, double ydelta) {
    static double ypos = 0;
    ypos += ydelta;
    if (ypos < -SCROLL_THRESHOLD) {
        item_index = (item_index + 1) % item_count;
        ypos = 0;
    }
    if (ypos > SCROLL_THRESHOLD) {
        item_index--;
        if (item_index < 0) {
            item_index = item_count - 1;
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
    #ifdef _WIN32
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
    #endif
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
        client_version(1);
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

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glLogicOp(GL_INVERT);
    glClearColor(0, 0, 0, 1);

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

    int loaded = db_load_state(&x, &y, &z, &rx, &ry);
    ensure_chunks(x, y, z, 1);
    if (!loaded) {
        y = highest_block(x, z) + 2;
    }

    scale = get_scale_factor();
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
            if (INVERT_MOUSE) {
                ry += (my - py) * m;
            }
            else {
                ry -= (my - py) * m;
            }
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
        int sz = 0;
        int sx = 0;
        if (!typing) {
            float m = dt * 1.0;
            ortho = glfwGetKey(window, CRAFT_KEY_ORTHO) ? 64 : 0;
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
        float vx, vy, vz;
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
        float speed = flying ? 20 : 5;
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

        // HANDLE CLICKS //
        if (left_click) {
            left_click = 0;
            int hx, hy, hz;
            int hw = hit_test(0, x, y, z, rx, ry, &hx, &hy, &hz);
            if (hy > 0 && hy < 256 && is_destructable(hw)) {
                set_block(hx, hy, hz, 0);
                int above = get_block(hx, hy + 1, hz);
                if (is_plant(above)) {
                    set_block(hx, hy + 1, hz, 0);
                }
            }
        }
        if (right_click) {
            right_click = 0;
            int hx, hy, hz;
            int hw = hit_test(1, x, y, z, rx, ry, &hx, &hy, &hz);
            if (hy > 0 && hy < 256 && is_obstacle(hw)) {
                if (!player_intersects_block(2, x, y, z, hx, hy, hz)) {
                    set_block(hx, hy, hz, items[item_index]);
                }
            }
        }
        if (middle_click) {
            middle_click = 0;
            int hx, hy, hz;
            int hw = hit_test(0, x, y, z, rx, ry, &hx, &hy, &hz);
            for (int i = 0; i < item_count; i++) {
                if (items[i] == hw) {
                    item_index = i;
                    break;
                }
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
            char format[64];
            snprintf(
                format, sizeof(format), "N,%%d,%%%ds", MAX_NAME_LENGTH - 1);
            char name[MAX_NAME_LENGTH];
            if (sscanf(buffer, format, &pid, name) == 2) {
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
            if (sscanf(buffer, format,
                &bp, &bq, &bx, &by, &bz, &face, text) >= 6)
            {
                _set_sign(bp, bq, bx, by, bz, face, text);
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
        if (SHOW_ITEM) {
            render_item(&block_attrib);
        }

        // RENDER TEXT //
        char text_buffer[1024];
        float ts = 12 * scale;
        float tx = ts / 2;
        float ty = height - ts;
        if (SHOW_INFO_TEXT) {
            int hour = time_of_day() * 24;
            char am_pm = hour < 12 ? 'a' : 'p';
            hour = hour % 12;
            hour = hour ? hour : 12;
            snprintf(
                text_buffer, 1024,
                "(%d, %d) (%.2f, %.2f, %.2f) [%d, %d, %d] %d%cm %dfps",
                chunked(x), chunked(z), x, y, z,
                player_count, chunk_count,
                face_count * 2, hour, am_pm, fps.fps);
            render_text(&text_attrib, LEFT, tx, ty, ts, text_buffer);
            ty -= ts * 2;
        }
        if (SHOW_CHAT_TEXT) {
            for (int i = 0; i < MAX_MESSAGES; i++) {
                int index = (message_index + i) % MAX_MESSAGES;
                if (strlen(messages[index])) {
                    render_text(&text_attrib, LEFT, tx, ty, ts,
                        messages[index]);
                    ty -= ts * 2;
                }
            }
        }
        if (typing) {
            snprintf(text_buffer, 1024, "> %s", typing_buffer);
            render_text(&text_attrib, LEFT, tx, ty, ts, text_buffer);
            ty -= ts * 2;
        }
        if (SHOW_PLAYER_NAMES) {
            if (player != me) {
                render_text(&text_attrib, CENTER, width / 2, ts, ts,
                    player->name);
            }
            Player *other = player_crosshair(player);
            if (other) {
                render_text(&text_attrib, CENTER,
                    width / 2, height / 2 - ts - 24, ts, other->name);
            }
        }

        // RENDER PICTURE IN PICTURE //
        if (observe2) {
            player = players + observe2;

            int pw = 256 * scale;
            int ph = 256 * scale;
            int offset = 32 * scale;
            int pad = 3 * scale;
            int sw = pw + pad * 2;
            int sh = ph + pad * 2;

            glEnable(GL_SCISSOR_TEST);
            glScissor(width - sw - offset + pad, offset - pad, sw, sh);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(width - pw - offset, offset, pw, ph);

            width = pw;
            height = ph;
            ortho = 0;
            fov = 65;

            render_sky(&sky_attrib, player, sky_buffer);
            glClear(GL_DEPTH_BUFFER_BIT);
            render_chunks(&block_attrib, player);
            render_signs(&text_attrib, player);
            render_players(&block_attrib, player);
            glClear(GL_DEPTH_BUFFER_BIT);
            if (SHOW_PLAYER_NAMES) {
                render_text(&text_attrib, CENTER, pw / 2, ts, ts,
                    player->name);
            }
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
