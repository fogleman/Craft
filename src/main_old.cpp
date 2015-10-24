#include<nanogui/nanogui.h>
#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
#else
  #include <arpa/inet.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include "client.h"
#include "config.h"
#include "cube.h"
#include "item.h"
#include "matrix.h"
#include "noise/noise.h"
#include "util.h"
#include "inventory.h"
#include "compress.h"
#include "crypto.h"
#include "konstructs.h"

Model model;
Model *g = &model;
MoveItem move_item;

void init_chunk(Chunk *chunk, int p, int q, int k);
void place_block_global_cords(int x, int y, int z, int w);

int chunked_int(int p) {
    if(p < 0) {
        return (p - CHUNK_SIZE + 1) / CHUNK_SIZE;
    } else {
        return p / CHUNK_SIZE;
    }
}

int chunked(float p) {
    return chunked_int(roundf(p));
}

int is_connected() {
    if (g->server_addr[0] == '\0'
        || g->server_user[0] == '\0'
        || g->server_pass[0] == '\0')
        return 0;
    return 1;
}

void update_login_prompt() {
    g->text_message[0] = '\0';
    g->text_prompt[0] = '\0';
    if (strlen(g->server_addr) == 0) {
        // must enter the address
        snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "Enter the server address");
        snprintf(g->text_prompt, KONSTRUCTS_TEXT_MESSAGE_SIZE, "Server");
    } else if (strlen(g->server_addr) > 0 && strlen(g->server_user) == 0) {
        // must enter the username
        snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "Enter your username to login/register");
        snprintf(g->text_prompt, KONSTRUCTS_TEXT_MESSAGE_SIZE, "User");
    } else if (strlen(g->server_user) > 0 && strlen(g->server_pass) == 0) {
        // must enter the password
        snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "Enter your password to login/register");
        snprintf(g->text_prompt, KONSTRUCTS_TEXT_MESSAGE_SIZE, "Password");
    } else {
        g->typing = 0;
        snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "Entering server %s as %s", g->server_addr, g->server_user);
        snprintf(g->text_prompt, KONSTRUCTS_TEXT_MESSAGE_SIZE, "Chat");
    }

}

void connect_console_command(char *str) {
    if (g->server_addr[0] == '\0') {
        if (!check_server(str)) {
            snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "Failed to resolve %s, try again.\n", str);
            return;
        }
        strncpy(g->server_addr, str, MAX_ADDR_LENGTH);
        update_login_prompt();
        g->typing = 1;
        g->typing_buffer[0] = '\0';
    } else if (g->server_user[0] == '\0') {
        char new_str[strlen(str)+1]; // Strip all whitespaces away
        int j = 0;
        for(int i = 0; i <= strlen(str); i++) {
            if(str[i] == ' ') {
                continue;
            }
            if(str[i] == '\0') {
                new_str[j] = '\0';
                break;
            }
            new_str[j++] = str[i];
        }
        if(new_str[0] != '\0') { // Success, the string is valid
            strncpy(g->server_user, str, 32);
            update_login_prompt();
        } else { // Failed, the string is empty or contains only whitespaces
            snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                    "The username cannot be empty!");
        }
        g->typing = 1;
        g->typing_buffer[0] = '\0';
    } else if (g->server_pass[0] == '\0') {
        char new_str[strlen(str)+1]; // Strip all whitespaces away
        int j = 0;
        for(int i = 0; i <= strlen(str); i++) {
            if(str[i] == ' ') {
                continue;
            }
            if(str[i] == '\0') {
                new_str[j] = '\0';
                break;
            }
            new_str[j++] = str[i];
        }
        if(new_str[0] != '\0') { // Success, the string is valid
            strncpy(g->server_pass, str, 64);
            update_login_prompt();
        } else { // Failed, the string is empty or contains only whitespaces
            snprintf(g->text_message, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                    "The password cannot be empty!");
        }
        g->typing = 0;
        g->typing_buffer[0] = '\0';
    }
}

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
    float x = g->width / 2;
    float y = g->height / 2;
    float p = 10 * g->scale;
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

Chunk *find_chunk(int p, int q, int k, int init) {
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk->p == p && chunk->q == q && chunk->k == k) {
            return chunk;
        }
    }
    if(init) {
        if (g->chunk_count < MAX_CHUNKS) {
            Chunk *chunk = g->chunks + g->chunk_count++;
            init_chunk(chunk, p, q, k);
            return chunk;
        }
    }
    return 0;
}

int chunk_distance(Chunk *chunk, int p, int q, int k) {
    int dp = ABS(chunk->p - p);
    int dq = ABS(chunk->q - q);
    int dk = ABS(chunk->k - k);
    return MAX(MAX(dp, dq), dk);
}

int chunk_near_player(int p, int q, int k, State *s, float distance) {
    if(p == chunked(s->x - distance) || p == chunked(s->x + distance)) {
        return 1;
    } else if(q == chunked(s->z - distance) || q == chunked(s->z + distance)) {
        return 1;
    } else if(k == chunked(s->y - distance) || k == chunked(s->y + distance)) {
        return 1;
    } else {
        return 0;
    }
}

int chunk_visible(float planes[6][4], int p, int q, int k) {
    float x = p * CHUNK_SIZE - 1;
    float z = q * CHUNK_SIZE - 1;
    float y = k * CHUNK_SIZE - 1;
    float d = CHUNK_SIZE + 1;
    float points[8][3] = {
        {x + 0, y + 0, z + 0},
        {x + d, y + 0, z + 0},
        {x + 0, y + 0, z + d},
        {x + d, y + 0, z + d},
        {x + 0, y + d, z + 0},
        {x + d, y + d, z + 0},
        {x + 0, y + d, z + d},
        {x + d, y + d, z + d}
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

int _hit_test(
    Chunk *chunk, float max_distance, int previous,
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
            int hw = chunk_get(chunk, nx, ny, nz);
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
    int k = chunked(y);
    float vx, vy, vz;
    get_sight_vector(rx, ry, &vx, &vy, &vz);

    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk_distance(chunk, p, q, k) > 1) {
            continue;
        }
        int hx, hy, hz;
        int hw = _hit_test(chunk, 8, previous,
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
    if (is_obstacle[w]) {
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
    int k = chunked(*y);
    int nx = roundf(*x);
    int ny = roundf(*y);
    int nz = roundf(*z);
    float px = *x - nx;
    float py = *y - ny;
    float pz = *z - nz;
    float pad = 0.25;
    int r = 1;
    for (int dp = -r; dp <= r; dp++) {
        for (int dq = -r; dq <= r; dq++) {
            for (int dk = -r; dk <= r; dk++) {
              Chunk *chunk = find_chunk(p + dp, q + dq, k + dk, 0);
                if (!chunk) {
                    continue;
                }
                for (int dy = 0; dy < height; dy++) {
                    if (px < -pad && is_obstacle[chunk_get(chunk, nx - 1, ny - dy, nz)]) {
                        *x = nx - pad;
                    }
                    if (px > pad && is_obstacle[chunk_get(chunk, nx + 1, ny - dy, nz)]) {
                        *x = nx + pad;
                    }
                    if (py < -pad && is_obstacle[chunk_get(chunk, nx, ny - dy - 1, nz)]) {
                        *y = ny - pad;
                        result = 1;
                    }
                    if (py > pad && is_obstacle[chunk_get(chunk, nx, ny - dy + 1, nz)]) {
                        *y = ny + pad;
                        result = 1;
                    }
                    if (pz < -pad && is_obstacle[chunk_get(chunk, nx, ny - dy, nz - 1)]) {
                        *z = nz - pad;
                    }
                    if (pz > pad && is_obstacle[chunk_get(chunk, nx, ny - dy, nz + 1)]) {
                        *z = nz + pad;
                    }
                }
            }
        }
    }

    struct timeval curtime;
    gettimeofday(&curtime, NULL);

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

void occlusion(
    char neighbors[27], float shades[27],
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
            for (int k = 0; k < 4; k++) {
                shade_sum += shades[lookup4[i][j][k]];
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
#define XYZ(x, y, z) ((y) * XZ_SIZE * XZ_SIZE + (x) * XZ_SIZE + (z))
#define XZ(x, z) ((x) * XZ_SIZE + (z))

void compute_chunk(WorkerItem *item) {
    char *opaque = (char *)calloc(XZ_SIZE * XZ_SIZE * XZ_SIZE, sizeof(char));
    char *highest = (char *)calloc(XZ_SIZE * XZ_SIZE, sizeof(char));

    int ox = - CHUNK_SIZE - 1;
    int oy = - CHUNK_SIZE - 1;
    int oz = - CHUNK_SIZE - 1;


    /* Populate the opaque array with the chunk itself */
    char *blocks = item->neighbour_blocks[1][1][1];

    CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH;

    /* With the six sides of the chunk */

    /* Populate the opaque array with the chunk below */
    CHUNK_FOR_EACH_XZ(item->neighbour_blocks[1][1][0], CHUNK_SIZE - 1, ex, ey, ez, ew) {
        int x = ex - ox;
        int y = ey - CHUNK_SIZE - oy;
        int z = ez - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_2D;


    /* Populate the opaque array with the chunk above
     * The shading requires additional 8 blocks
     */
    for(int i = 0; i < 8; i++) {
        CHUNK_FOR_EACH_XZ(item->neighbour_blocks[1][1][2], i, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_2D;
    }

    /* Populate the opaque array with the chunk left */
    CHUNK_FOR_EACH_YZ(item->neighbour_blocks[0][1][1], CHUNK_SIZE - 1, ex, ey, ez, ew) {
        int x = ex - CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_2D;


    /* Populate the opaque array with the chunk right */
    CHUNK_FOR_EACH_YZ(item->neighbour_blocks[2][1][1], 0, ex, ey, ez, ew) {
        int x = ex + CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_2D;


    /* Populate the opaque array with the chunk front */
    CHUNK_FOR_EACH_XY(item->neighbour_blocks[1][0][1], CHUNK_SIZE - 1, ex, ey, ez, ew) {
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_2D;


    /* Populate the opaque array with the chunk back */
    CHUNK_FOR_EACH_XY(item->neighbour_blocks[1][2][1], 0, ex, ey, ez, ew) {
        int x = ex - ox;
        int y = ey - oy;
        int z = ez + CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_2D;

    /* Populate the corner cases above
     * Shading yet again requires 8 additional blocks
     */

    for(int i = 0; i < 8; i++) {
        /* Populate the opaque array with the chunk above-left */
        CHUNK_FOR_EACH_Z(item->neighbour_blocks[0][1][2], CHUNK_SIZE - 1, i, ex, ey, ez, ew) {
            int x = ex - CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the chunk above-right */
        CHUNK_FOR_EACH_Z(item->neighbour_blocks[2][1][2], 0, i, ex, ey, ez, ew) {
            int x = ex + CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the chunk above-front */
        CHUNK_FOR_EACH_X(item->neighbour_blocks[1][0][2], CHUNK_SIZE - 1, i, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;
        /* Populate the opaque array with the chunk above-back */
        CHUNK_FOR_EACH_X(item->neighbour_blocks[1][2][2], 0, i, ex, ey, ez, ew) {
            int x = ex - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = ew;
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        } END_CHUNK_FOR_EACH_1D;

        /* Populate the opaque array with the block above-left-front */
        {
            int ex = CHUNK_SIZE - 1;
            int ey = i;
            int ez = CHUNK_SIZE - 1;
            int x = ex - CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = item->neighbour_blocks[0][0][2][ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        }

        /* Populate the opaque array with the block above-right-front */
        {
            int ex = 0;
            int ey = i;
            int ez = CHUNK_SIZE - 1;
            int x = ex + CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez - CHUNK_SIZE - oz;
            int w = item->neighbour_blocks[2][0][2][ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        }

        /* Populate the opaque array with the block above-left-back */
        {
            int ex = CHUNK_SIZE - 1;
            int ey = i;
            int ez = 0;
            int x = ex - CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = item->neighbour_blocks[0][2][2][ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        }

        /* Populate the opaque array with the block above-right-back */
        {
            int ex = 0;
            int ey = i;
            int ez = 0;
            int x = ex + CHUNK_SIZE - ox;
            int y = ey + CHUNK_SIZE - oy;
            int z = ez + CHUNK_SIZE - oz;
            int w = item->neighbour_blocks[2][2][2][ex+ey*CHUNK_SIZE+ez*CHUNK_SIZE*CHUNK_SIZE];
            opaque[XYZ(x, y, z)] = !is_transparent[w];
            if (opaque[XYZ(x, y, z)]) {
                highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
            }
        }

    }

    /* Populate the corner cases on the same level */

    /* Populate the opaque array with the chunk left-front */
    CHUNK_FOR_EACH_Y(item->neighbour_blocks[0][0][1], CHUNK_SIZE - 1, CHUNK_SIZE - 1, ex, ey, ez, ew) {
        int x = ex - CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez - CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_1D;

    /* Populate the opaque array with the chunk left-back */
    CHUNK_FOR_EACH_Y(item->neighbour_blocks[0][2][1], CHUNK_SIZE - 1, 0, ex, ey, ez, ew) {
        int x = ex - CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez + CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_1D;

    /* Populate the opaque array with the chunk right-front */
    CHUNK_FOR_EACH_Y(item->neighbour_blocks[2][0][1], 0, CHUNK_SIZE - 1, ex, ey, ez, ew) {
        int x = ex + CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez - CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_1D;

    /* Populate the opaque array with the chunk right-back */
    CHUNK_FOR_EACH_Y(item->neighbour_blocks[2][2][1], 0, 0, ex, ey, ez, ew) {
        int x = ex + CHUNK_SIZE - ox;
        int y = ey - oy;
        int z = ez + CHUNK_SIZE - oz;
        int w = ew;
        opaque[XYZ(x, y, z)] = !is_transparent[w];
        if (opaque[XYZ(x, y, z)]) {
            highest[XZ(x, z)] = MAX(highest[XZ(x, z)], y);
        }
    } END_CHUNK_FOR_EACH_1D;


    // count exposed faces
    int faces = 0;
    CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
        if (ew <= 0) {
            continue;
        }
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - oz;
        int f1 = !opaque[XYZ(x - 1, y, z)];
        int f2 = !opaque[XYZ(x + 1, y, z)];
        int f3 = !opaque[XYZ(x, y + 1, z)];
        int f4 = !opaque[XYZ(x, y - 1, z)];
        int f5 = !opaque[XYZ(x, y, z - 1)];
        int f6 = !opaque[XYZ(x, y, z + 1)];
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        if (is_plant[ew]) {
            total = 4;
        }
        faces += total;
    } END_CHUNK_FOR_EACH;

    // generate geometry
    GLfloat *data = malloc_faces(10, faces);
    int offset = 0;
    CHUNK_FOR_EACH(blocks, ex, ey, ez, ew) {
        if (ew <= 0) {
            continue;
        }
        int x = ex - ox;
        int y = ey - oy;
        int z = ez - oz;
        int f1 = !opaque[XYZ(x - 1, y, z)];
        int f2 = !opaque[XYZ(x + 1, y, z)];
        int f3 = !opaque[XYZ(x, y + 1, z)];
        int f4 = !opaque[XYZ(x, y - 1, z)];
        int f5 = !opaque[XYZ(x, y, z - 1)];
        int f6 = !opaque[XYZ(x, y, z + 1)];
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        char neighbors[27] = {0};
        float shades[27] = {0};
        int index = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -1; dz <= 1; dz++) {
                    neighbors[index] = opaque[XYZ(x + dx, y + dy, z + dz)];
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
        occlusion(neighbors, shades, ao, light);
        if (is_plant[ew]) {
            total = 4;
            float min_ao = 1;
            float max_light = 0;
            for (int a = 0; a < 6; a++) {
                for (int b = 0; b < 4; b++) {
                    min_ao = MIN(min_ao, ao[a][b]);
                }
            }
            float rotation = simplex2(ex, ez, 4, 0.5, 2) * 360;
            make_plant(
                data + offset, min_ao, 0,
                ex, ey, ez, 0.5, ew, rotation);
        }
        else {
            make_cube(
                data + offset, ao, light,
                f1, f2, f3, f4, f5, f6,
                ex, ey, ez, 0.5, ew);
        }
        offset += total * 60;
    } END_CHUNK_FOR_EACH;

    free(opaque);
    free(highest);

    item->faces = faces;
    item->data = data;
}

void generate_chunk(Chunk *chunk, WorkerItem *item) {
    chunk->faces = item->faces;
    del_buffer(chunk->buffer);
    chunk->buffer = gen_faces(10, item->faces, item->data);
}

void gen_chunk_buffer(Chunk *chunk) {
    WorkerItem _item;
    WorkerItem *item = &_item;
    item->p = chunk->p;
    item->q = chunk->q;
    item->k = chunk->k;

    for (int dp = -1; dp <= 1; dp++) {
        for (int dq = -1; dq <= 1; dq++) {
            for (int dk = -1; dk <= 1; dk++) {
                /* Only the middle chunk is required below */
                if(dk == -1 && dq != 0 && dp != 0) continue;

                Chunk *other = chunk;
                if (dp || dq || dk) {
                    other = find_chunk(chunk->p + dp, chunk->q + dq, chunk->k + dk, 0);
                }
                if (other) {
                    memcpy(item->neighbour_blocks[dp + 1][dq + 1][dk + 1], other->blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                }
                else {
                    memset(item->neighbour_blocks[dp + 1][dq + 1][dk + 1], 0, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                }
            }
        }
    }
    compute_chunk(item);
    generate_chunk(chunk, item);
    chunk->dirty = 0;
}

 void init_chunk(Chunk *chunk, int p, int q, int k) {
    chunk->p = p;
    chunk->q = q;
    chunk->k = k;
    chunk->faces = 0;
    chunk->buffer = 0;
    chunk->dirty = 1;
    memset(chunk->blocks, 0, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
}

void delete_chunks() {
    int count = g->chunk_count;
    State *s1 = &g->players->state;
    State *s2 = &(g->players + g->observe1)->state;
    State *s3 = &(g->players + g->observe2)->state;
    State *states[3] = {s1, s2, s3};
    for (int i = 0; i < count; i++) {
        Chunk *chunk = g->chunks + i;
        int _delete = 1;
        for (int j = 0; j < 3; j++) {
            State *s = states[j];
            int p = chunked(s->x);
            int q = chunked(s->z);
            int k = chunked(s->y);
            if (chunk_distance(chunk, p, q, k) < g->delete_radius) {
                _delete = 0;
                break;
            }
        }
        if (_delete) {
            del_buffer(chunk->buffer);
            Chunk *other = g->chunks + (--count);
            memcpy(chunk, other, sizeof(Chunk));
        }
    }
    g->chunk_count = count;
}

void delete_all_chunks() {
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        del_buffer(chunk->buffer);
    }
    g->chunk_count = 0;
}

void check_workers() {
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        //mtx_lock(&worker->mtx);
        if (worker->state == WORKER_DONE) {
            WorkerItem *item = &worker->item;
            Chunk *chunk = find_chunk(item->p, item->q, item->k, 0);
            if (chunk) {
                generate_chunk(chunk, item);
            }
            worker->state = WORKER_IDLE;
        }
        //mtx_unlock(&worker->mtx);
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
    int k = chunked(s->y);
    int r = g->create_radius;
    int start = 0x0fffffff;
    int best_score = start;
    Chunk *best_chunk;
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        int a = chunk->p;
        int b = chunk->q;
        int c = chunk->k;
        int index = (ABS(a) ^ ABS(b) ) % WORKERS;
        if (index != worker->index) {
            continue;
        }
        if (chunk && !chunk->dirty) {
            continue;
        }
        int distance = MAX(ABS(p - a), MAX(ABS(q - b), ABS(k - c)));
        int invisible = !(chunk_near_player(p, q, k, s, 10) || chunk_visible(planes, a, b, c));
        int priority = 0;
        priority = chunk->buffer && chunk->dirty;
        int score = (invisible << 24) | (priority << 16) | distance;
        if (score < best_score) {
            best_score = score;
            best_chunk = chunk;
        }
    }
    if (best_score == start) {
        return;
    }

    Chunk *chunk = best_chunk;
    WorkerItem *item = &worker->item;
    item->p = chunk->p;
    item->q = chunk->q;
    item->k = chunk->k;
    for (int dp = -1; dp <= 1; dp++) {
        for (int dq = -1; dq <= 1; dq++) {
            for (int dk = -1; dk <= 1; dk++) {
                /* Only the middle chunk is required below */
                if(dk == -1 && dq != 0 && dp != 0) continue;
                Chunk *other = chunk;
                if (dp || dq || dk) {
                    other = find_chunk(chunk->p + dp, chunk->q + dq, chunk->k + dk, 0);
                }
                if (other) {
                    memcpy(item->neighbour_blocks[dp + 1][dq + 1][dk + 1], other->blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                }
                else {
                    memset(item->neighbour_blocks[dp + 1][dq + 1][dk + 1], 0, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                  }
            }
        }
    }
    chunk->dirty = 0;
    worker->state = WORKER_BUSY;
    //cnd_signal(&worker->cnd);
}

void ensure_chunks(Player *player) {
    check_workers();
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        //mtx_lock(&worker->mtx);
        if (worker->state == WORKER_IDLE) {
            ensure_chunks_worker(player, worker);
        }
        //mtx_unlock(&worker->mtx);
    }
}

int worker_run(void *arg) {
    Worker *worker = (Worker *)arg;
    int running = 1;
    while (running) {
        //mtx_lock(&worker->mtx);
        while (worker->state != WORKER_BUSY) {
            //cnd_wait(&worker->cnd, &worker->mtx);
        }
        //mtx_unlock(&worker->mtx);
        WorkerItem *item = &worker->item;
        compute_chunk(item);
        //mtx_lock(&worker->mtx);
        worker->state = WORKER_DONE;
        //mtx_unlock(&worker->mtx);
    }
    return 0;
}

int render_chunks(Attrib *attrib, Player *player) {
    int result = 0;
    State *s = &player->state;
    ensure_chunks(player);
    int p = chunked(s->x);
    int q = chunked(s->z);
    int k = chunked(s->y);
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
    for (int i = 0; i < g->chunk_count; i++) {
        Chunk *chunk = g->chunks + i;
        if (chunk_distance(chunk, p, q, k) > g->render_radius) {
            continue;
        }
        if (!chunk_visible(planes, chunk->p, chunk->q, chunk->k))
        {
            continue;
        }

        float rx = 0;
        float ry = 0;
        float translation[16];
        float tmp[16];
        mat_identity(translation);
        mat_translate(tmp, (float)chunk->p * CHUNK_SIZE, (float)chunk->k * CHUNK_SIZE, (float)chunk->q * CHUNK_SIZE);
        mat_multiply(translation, tmp, translation);
        mat_rotate(tmp, 0, 1, 0, rx);
        mat_multiply(translation, tmp, translation);
        mat_rotate(tmp, cosf(rx), 0, sinf(rx), -ry);
        mat_multiply(translation, tmp, translation);
        glUniformMatrix4fv(attrib->extra5, 1, GL_FALSE, translation);
        draw_chunk(attrib, chunk);
        result += chunk->faces;
    }
    return result;
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
    float identity[16];
    mat_identity(identity);
    glUniformMatrix4fv(attrib->extra5, 1, GL_FALSE, identity);
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
    if (is_obstacle[hw]) {
        glUseProgram(attrib->program);
        glLineWidth(2);
        glEnable(GL_COLOR_LOGIC_OP);
        glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
        glUniform4f(attrib->extra1, 0.0, 0.0, 0.0, 1.0);
        GLuint wireframe_buffer = gen_wireframe_buffer(hx, hy, hz, 0.52);
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
    glUniform4f(attrib->extra1, 0.0, 0.0, 0.0, 1.0);
    GLuint crosshair_buffer = gen_crosshair_buffer();
    draw_lines(attrib, crosshair_buffer, 2, 4);
    del_buffer(crosshair_buffer);
    glDisable(GL_COLOR_LOGIC_OP);
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

void print(Attrib *attrib, int justify, float x, float y, float n, char *text) {
    int length = strlen(text);
    x -= n * justify * (length - 1) / 2;
    GLuint buffer = gen_text_buffer(x, y, n, text);
    draw_text(attrib, buffer, length);
    del_buffer(buffer);
}

void add_message(const char *text) {
    printf("%s\n", text);
    snprintf(
        g->messages[g->message_index], MAX_TEXT_LENGTH, "%s", text);
    g->message_index = (g->message_index + 1) % MAX_MESSAGES;
}

void on_left_click() {

    if(g->inventory_screen) {
        double xpos, ypos;
        glfwGetCursorPos(g->window, &xpos, &ypos);

        // Scale factor for the boxes
        float s = 0.12 * WINDOW_WIDTH/g->width;
        float v = 0.12 * WINDOW_HEIGHT/g->height;

        // Position on the screen in glcoords
        float gl_x = (xpos/g->width * 2 - 1);
        float gl_y = (ypos/g->height * 2 - 1);

        // Calc offset (if the window is resized)
        gl_y = gl_y + -1 * ((float)g->height - EXT_INVENTORY_PX_FROM_BOTTOM)/(float)g->height
               + v*EXT_INVENTORY_ROWS/2;

        // Get selected col/row
        int col = (gl_x + EXT_INVENTORY_COLS * s)/s - EXT_INVENTORY_COLS/2;
        int row = (gl_y + EXT_INVENTORY_ROWS * v)/v - EXT_INVENTORY_ROWS/2;

        // The inventory is rendered upside down so...
        row = EXT_INVENTORY_ROWS - 1 - row;

        // Our inventory position
        int index = row*EXT_INVENTORY_COLS + col;

        // Ignore to large/small selections
        if(index < 0 || index > EXT_INVENTORY_ROWS*EXT_INVENTORY_COLS) return;

        // We have something selected and a slot is selected
        if (move_item.use == 1) {
            client_click_inventory(index);
            move_item.use = 0;
            if (DEBUG) printf("Inventory: Move slot %d to %d\n", move_item.index, index);
        // Select a item, if there is a item in the slot
        } else {
            client_click_inventory(index);
            move_item.index = index;
            move_item.use = 1;
            if(g->mouse_item == -1) {
              g->mouse_item = ext_inventory.items[index].id;
              ext_inventory.items[index].id = 0;
            }
            if (DEBUG) printf("Inventory: Select slot %d\n", index);
        }

        return;
    }

    State *s = &g->players->state;
    int hx, hy, hz;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    click_at(hw > 0 ? 1 : 0, hx, hy, hz, 1);
}

void on_right_click() {
    State *s = &g->players->state;
    int hx, hy, hz, w;
    int hw = hit_test(1, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    if (hw > 0 && !player_intersects_block(2, s->x, s->y, s->z, hx, hy, hz)) {
        click_at(1, hx, hy, hz, 2);
        if (inventory.selected >= 0) {
            w = inventory.items[inventory.selected].id;
            if (w > 0) {
                place_block_global_cords(hx, hy, hz, w);
            }
        }
    } else {
        click_at(0, 0, 0, 0, 2);
    }

}

void on_middle_click() {
    State *s = &g->players->state;
    int hx = 0, hy = 0, hz = 0;
    int hw = hit_test(0, s->x, s->y, s->z, s->rx, s->ry, &hx, &hy, &hz);
    click_at(hw > 0 ? 1 : 0, hx, hy, hz, 3);
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
        if (g->typing && is_connected()) {
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
                if (!is_connected()) {
                    connect_console_command(g->typing_buffer);
                } else {
                    client_talk(g->typing_buffer);
                    g->typing = 0;
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
    }
    if (!g->typing) {
        if (key == KONSTRUCTS_KEY_FLY) {
            g->flying = !g->flying;
        }
        if (key == KONSTRUCTS_KEY_DEBUG_SCREEN) {
            g->debug_screen = !g->debug_screen;
        }
        if (key == KONSTRUCTS_KEY_ACTIVATE_ITEM) {
            if (!g->inventory_screen) {
                on_middle_click();
            } else {
                g->inventory_screen = 0;
                glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                client_close_inventory();
            }
        }
        if (key == KONSTRUCTS_KEY_CLOSE) {
            if(g->inventory_screen) {
                g->inventory_screen = 0;
                glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                client_close_inventory();
            }
        }
        if (key == KONSTRUCTS_KEY_INVENTORY_KONSTRUCT) {
            if(g->inventory_screen) {
                client_konstruct();
            }
        }
        if (key == KONSTRUCTS_KEY_OBSERVE) {
            g->observe1 = (g->observe1 + 1) % g->player_count;
        }
        if (key == KONSTRUCTS_KEY_OBSERVE_INSET) {
            g->observe2 = (g->observe2 + 1) % g->player_count;
        }
    }
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
        if (u == KONSTRUCTS_KEY_CHAT) {
            g->typing = 1;
            g->typing_buffer[0] = '\0';
        }
        if (u == KONSTRUCTS_KEY_COMMAND) {
            g->typing = 1;
            g->typing_buffer[0] = '/';
            g->typing_buffer[1] = '\0';
        }
        if (u > 48 && u < 58) {
            inventory.selected = (int)u - 49;
            client_inventory_select((int)u - 49);
        }
    }
}

void on_scroll(GLFWwindow *window, double xdelta, double ydelta) {
    static double ypos = 0;
    ypos += ydelta;
    if (ypos < -SCROLL_THRESHOLD) {
        if (INVENTORY_SLOTS > inventory.selected + 1)
            inventory.selected = inventory.selected + 1;
            client_inventory_select(inventory.selected);
        ypos = 0;
    } else if (ypos > SCROLL_THRESHOLD) {
        if (inventory.selected > 0)
            inventory.selected = inventory.selected - 1;
            client_inventory_select(inventory.selected);
        ypos = 0;
    }
}

void on_mouse_button(GLFWwindow *window, int button, int action, int mods) {
    int control = mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER);
    int exclusive =
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

    // Make sure that a mouse button was pressed
    if (action != GLFW_PRESS) return;

    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:

            if (!exclusive) {
                if(g->inventory_screen) {
                    on_left_click();
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                break;
            }

            if (control) {
                on_right_click();
            } else {
                on_left_click();
            }

            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            if (exclusive) on_right_click();
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            if (exclusive) on_middle_click();
            break;
    }

}

void create_window() {
    GLFWmonitor *monitor = NULL;
    g->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Konstructs", monitor, NULL);
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
        g->ortho = glfwGetKey(g->window, KONSTRUCTS_KEY_ORTHO) ? 64 : 0;
        g->fov = glfwGetKey(g->window, KONSTRUCTS_KEY_ZOOM) ? 15 : 65;
        if (glfwGetKey(g->window, KONSTRUCTS_KEY_FORWARD)) sz--;
        if (glfwGetKey(g->window, KONSTRUCTS_KEY_BACKWARD)) sz++;
        if (glfwGetKey(g->window, KONSTRUCTS_KEY_LEFT)) sx--;
        if (glfwGetKey(g->window, KONSTRUCTS_KEY_RIGHT)) sx++;
        if (glfwGetKey(g->window, GLFW_KEY_LEFT)) s->rx -= m;
        if (glfwGetKey(g->window, GLFW_KEY_RIGHT)) s->rx += m;
        if (glfwGetKey(g->window, GLFW_KEY_UP)) s->ry += m;
        if (glfwGetKey(g->window, GLFW_KEY_DOWN)) s->ry -= m;
    }
    float vx, vy, vz;
    get_motion_vector(g->flying, sz, sx, s->rx, s->ry, &vx, &vy, &vz);
    if (!g->typing) {
        if (glfwGetKey(g->window, KONSTRUCTS_KEY_JUMP)) {
            if (g->flying) {
                vy = 1;
            }
            else if (dy == 0) {
                dy = 8;
            }
        }
    }
    float speed = g->flying ? 20 : 5;
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
        if (g->flying || !find_chunk(chunked(s->x), chunked(s->z), chunked(s->y), 0)) {
            dy = 0;
        }
        else {
            dy -= ut * 25;
            dy = MAX(dy, -250);
        }
        s->x += vx;
        s->y += vy + dy * ut;
        s->z += vz;
        if (collide(2, &s->x, &s->y, &s->z)) {
            dy = 0;
        }
    }
    if (s->y < 0) {
      s->y = 2;
    }
}

void parse_block(Chunk * chunk, int x, int y, int z, int w, State *s) {
    chunk_set(chunk, x, y, z, w);

    if (player_intersects_block(2, s->x, s->y, s->z, x, y, z)) {
        s->y += 2;
    }

}

void parse_blocks(int p, int q, int k, char* blocks, int size, State *s) {
    g->blocks_recv = g->blocks_recv + CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    int out_size = inflate_data(blocks + BLOCKS_HEADER_SIZE, size - BLOCKS_HEADER_SIZE,
                                g->chunk_buffer, g->chunk_buffer_size);
    Chunk *chunk = find_chunk(p, q, k, 1);
    if(chunk) {
        memcpy(chunk->blocks, g->chunk_buffer, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
        int r = 1;
        chunk->dirty = 1;
        /* Make all adjacent chunks dirty */
        for (int dp = -r; dp <= r; dp++) {
            for (int dq = -r; dq <= r; dq++) {
                for (int dk = -r; dk <= r; dk++) {
                    int a = chunk->p + dp;
                    int b = chunk->q + dq;
                    int c = chunk->k + dk;
                    Chunk *adj = find_chunk(a, b, c, 0);
                    if (adj) {
                        adj->dirty = 1;
                    }
                }
            }
        }
    }
}

void place_block(int bp, int bq, int bx, int by, int bz, int bw) {
    State *s = &g->players->state;
    int bk = chunked_int(by);
    Chunk *chunk = find_chunk(bp, bq, bk, 1);
    if(chunk) {
        parse_block(chunk,
                    bx, by,
                    bz, bw, s);
        chunk->dirty = 1;
        /* Make any chunk that owns an adjacent block dirty as well */
        int r = 1;
        for (int dx = -r; dx <= r; dx++) {
            for (int dz = -r; dz <= r; dz++) {
                for (int dy = -r; dy <= r; dy++) {
                    int x = bx + dx;
                    int y = by + dy;
                    int z = bz + dz;
                    int np = chunked_int(x);
                    int nq = chunked_int(z);
                    int nk = chunked_int(y);
                    Chunk *c = find_chunk(np, nq, nk, 0);
                    if (c) {
                        c->dirty = 1;
                    }
                }
            }
        }
        /* If the chunk is close to the player, force a chunk update */
        if(chunk_near_player(bp, bq, bk, s, 10)) {
            for (int dp = -r; dp <= r; dp++) {
                for (int dq = -r; dq <= r; dq++) {
                    for (int dk = -r; dk <= r; dk++) {
                        int np = bp + dp;
                        int nq = bq + dq;
                        int nk = bk + dk;
                        Chunk *c = find_chunk(np, nq, nk, 0);
                        if(c && c->dirty) {
                            gen_chunk_buffer(chunk);
                        }
                    }
                }
            }
        }
    }
}

void place_block_global_cords(int x, int y, int z, int w) {
    place_block(chunked_int(x), chunked_int(z), x, y, z, w);
}

void parse_buffer(Packet packet) {

    Player *me = g->players;
    State *s = &g->players->state;

    char *payload = packet.payload;
    int size = packet.size;
    char type = packet.type;
    if (type == 'C') {
    //    int bp, bq, bk;
    //    char *pos = payload;

    //    bp = ntohl(*((int*)pos));
    //    pos += sizeof(int);

    //    bq = ntohl(*((int*)pos));
    //    pos += sizeof(int);

    //    bk = ntohl(*((int*)pos));
    //    pos += sizeof(int);

    //    parse_blocks(bp, bq, bk, pos, size - sizeof(int)*3, s);
    //    client_chunk(1);
    } else if(type == 'M') {
        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        load_png_texture_from_buffer(payload, size);
    } else {
        char *line = (char*)malloc((size + 1) * sizeof(char));
        memcpy(line, payload, size);
        line[size] = '\0';

        if (DEBUG) printf("Proto[Line]: %c%s\n", type, line);

        if(type == 'W') {
            int w, obstacle, transparent, left, right, top, bottom, front, back;
            char shape[16];
            if(sscanf(line, ",%d,%15[^,],%d,%d,%d,%d,%d,%d,%d,%d",
                      &w, shape, &obstacle, &transparent, &left, &right,
                      &top, &bottom, &front, &back) == 10) {
                is_plant[w] = strncmp(shape, "plant", 16) == 0;
                is_obstacle[w] = obstacle;
                is_transparent[w] = transparent;
                blocks[w][0] = left;
                blocks[w][1] = right;
                blocks[w][2] = top;
                blocks[w][3] = bottom;
                blocks[w][4] = front;
                blocks[w][5] = back;
            }
        }

        if(type == 'U') {
            int pid;
            float ux, uy, uz, urx, ury;
            if (sscanf(line, ",%d,%f,%f,%f,%f,%f",
                       &pid, &ux, &uy, &uz, &urx, &ury) == 6)
                {
                    me->id = pid;
                    s->x = ux; s->y = uy; s->z = uz; s->rx = urx; s->ry = ury;
                    if (uy == 0) {
                        s->y = 200;
                    }
                }
        }
        if(type == 'I') {
            int pos, amount, id, inv;
            if (sscanf(line, ",%d,%d,%d", &pos, &amount, &id) == 3) {
                ext_inventory.items[pos].id = id;
                ext_inventory.items[pos].num = amount;
                ext_inventory.items[pos].show = id == -1 ? 0 : 1;
                glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                g->inventory_screen = 1;
            }
        }
        if (type == 'i') {
            int amount, id;
            if(sscanf(line, ",%d,%d", &amount, &id) == 2) {
                g->mouse_item = id;
            }
        }
        if (type == 'G') {
            int pos, id, amount;
            if(sscanf(line, ",%d,%d,%d", &pos, &amount, &id) == 3) {
                inventory.items[pos].id = id;
                inventory.items[pos].num = amount;
            }
        }
        if (type == 'A') {
            int id;
            if(sscanf(line, ",%d", &id) == 1) {
                inventory.selected = id;
            }
        }
        if(false && type == 'P') {
            int pid;
            float px, py, pz, prx, pry;
            if (sscanf(line, ",%d,%f,%f,%f,%f,%f",
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
        }
        if (type == 'D') {
            int pid;
            if(sscanf(line, ",%d", &pid) == 1) {
                delete_player(pid);
            }
        }
        if(type == 'E') {
            double elapsed;
            int day_length;
            if (sscanf(line, ",%lf,%d", &elapsed, &day_length) == 2) {
                glfwSetTime(fmod(elapsed, day_length));
                g->day_length = day_length;
                g->time_changed = 1;
            }
        }
        if (type == 'T' && line[0] == ',') {
            char *text = line + 1;
            add_message(text);
        }
        if(false && type == 'N') {
            char format[64];
            snprintf(
                     format, sizeof(format), ",%%d,%%%ds", MAX_NAME_LENGTH - 1);
            int pid;
            char name[MAX_NAME_LENGTH];
            if (sscanf(line, format, &pid, name) == 2) {
                Player *player = find_player(pid);
                if (player) {
                    strncpy(player->name, name, MAX_NAME_LENGTH);
                }
            }
        }
    }
}

void reset_model() {
    memset(g->chunks, 0, sizeof(Chunk) * MAX_CHUNKS);
    g->chunk_count = 0;
    memset(g->players, 0, sizeof(Player) * MAX_PLAYERS);
    g->player_count = 0;
    g->observe1 = 0;
    g->observe2 = 0;
    g->flying = 0;
    memset(g->typing_buffer, 0, sizeof(char) * MAX_TEXT_LENGTH);
    g->typing = 0;
    memset(g->messages, 0, sizeof(char) * MAX_MESSAGES * MAX_TEXT_LENGTH);
    g->message_index = 0;
    g->day_length = DAY_LENGTH;
    glfwSetTime(g->day_length / 3.0);
    g->time_changed = 1;
}

#ifdef _WIN32
int init_winsock() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }

    return 0;
}
#else
int init_winsock() { return 0; }
#endif

int init_inventory() {

    // player inventory/belt
    inventory.items = (Item*)calloc(INVENTORY_SLOTS * INVENTORY_ROWS, sizeof(Item));
    for (int item = 0; item < INVENTORY_SLOTS * INVENTORY_ROWS; item ++) {
        inventory.items[item].id = 0;
        inventory.items[item].num = 0;
        inventory.items[item].show = 1;
    }
    inventory.selected = 0;

    // external inventory
    ext_inventory.items = (Item*)calloc(EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS, sizeof(Item));
    for (int item = 0; item < EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS; item ++) {
        ext_inventory.items[item].id = 0;
        ext_inventory.items[item].num = 0;
        ext_inventory.items[item].show = 0;
    }
    ext_inventory.selected = -1;

}

void shtxt_path(const char *name, const char *type, char *path, size_t max_len) {
    snprintf(path, max_len, "%s/%s", type, name);

    if (!file_exist(path)) {
        snprintf(path, max_len, "/usr/local/share/konstructs-client/%s/%s", type, name);
    }

    if (!file_exist(path)) {
        printf("Error, no %s for %s found.\n", type, name);
        exit(1);
    }
}

void texture_path(const char *name, char *path, size_t max_len) {
    shtxt_path(name, "textures", path, max_len);
}

void shader_path(const char *name, char *path, size_t max_len) {
    shtxt_path(name, "shaders", path, max_len);
}

void load_textures() {
    char txtpth[KONSTRUCTS_PATH_SIZE];

    GLuint font;
    glGenTextures(1, &font);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_path("font.png", txtpth, KONSTRUCTS_PATH_SIZE);
    load_png_texture(txtpth);

    GLuint sky;
    glGenTextures(1, &sky);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, sky);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture_path("sky.png", txtpth, KONSTRUCTS_PATH_SIZE);
    load_png_texture(txtpth);

    GLuint inventory_texture;
    glGenTextures(1, &inventory_texture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, inventory_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    texture_path("inventory.png", txtpth, KONSTRUCTS_PATH_SIZE);
    load_png_texture(txtpth);

}

void load_shaders(Attrib *block_attrib, Attrib *line_attrib, Attrib *text_attrib,
        Attrib *sky_attrib, Attrib *inventory_attrib) {
    char vertex_path[KONSTRUCTS_PATH_SIZE];
    char fragment_path[KONSTRUCTS_PATH_SIZE];
    GLuint program;

    shader_path("block_vertex.glsl", vertex_path, KONSTRUCTS_PATH_SIZE);
    shader_path("block_fragment.glsl", fragment_path, KONSTRUCTS_PATH_SIZE);

    program = load_program(vertex_path, fragment_path);
    block_attrib->program = program;
    block_attrib->position = glGetAttribLocation(program, "position");
    block_attrib->normal = glGetAttribLocation(program, "normal");
    block_attrib->uv = glGetAttribLocation(program, "uv");
    block_attrib->matrix = glGetUniformLocation(program, "matrix");
    block_attrib->sampler = glGetUniformLocation(program, "sampler");
    block_attrib->extra1 = glGetUniformLocation(program, "sky_sampler");
    block_attrib->extra2 = glGetUniformLocation(program, "daylight");
    block_attrib->extra3 = glGetUniformLocation(program, "fog_distance");
    block_attrib->extra4 = glGetUniformLocation(program, "ortho");
    block_attrib->extra5 = glGetUniformLocation(program, "translation");
    block_attrib->camera = glGetUniformLocation(program, "camera");
    block_attrib->timer = glGetUniformLocation(program, "timer");

    shader_path("line_vertex.glsl", vertex_path, KONSTRUCTS_PATH_SIZE);
    shader_path("line_fragment.glsl", fragment_path, KONSTRUCTS_PATH_SIZE);

    program = load_program(vertex_path, fragment_path);
    line_attrib->program = program;
    line_attrib->position = glGetAttribLocation(program, "position");
    line_attrib->matrix = glGetUniformLocation(program, "matrix");
    line_attrib->extra1 = glGetUniformLocation(program, "color");

    shader_path("text_vertex.glsl", vertex_path, KONSTRUCTS_PATH_SIZE);
    shader_path("text_fragment.glsl", fragment_path, KONSTRUCTS_PATH_SIZE);

    program = load_program(vertex_path, fragment_path);
    text_attrib->program = program;
    text_attrib->position = glGetAttribLocation(program, "position");
    text_attrib->uv = glGetAttribLocation(program, "uv");
    text_attrib->matrix = glGetUniformLocation(program, "matrix");
    text_attrib->sampler = glGetUniformLocation(program, "sampler");

    shader_path("sky_vertex.glsl", vertex_path, KONSTRUCTS_PATH_SIZE);
    shader_path("sky_fragment.glsl", fragment_path, KONSTRUCTS_PATH_SIZE);

    program = load_program(vertex_path, fragment_path);
    sky_attrib->program = program;
    sky_attrib->position = glGetAttribLocation(program, "position");
    sky_attrib->normal = glGetAttribLocation(program, "normal");
    sky_attrib->uv = glGetAttribLocation(program, "uv");
    sky_attrib->matrix = glGetUniformLocation(program, "matrix");
    sky_attrib->sampler = glGetUniformLocation(program, "sampler");
    sky_attrib->timer = glGetUniformLocation(program, "timer");

    shader_path("inventory_vertex.glsl", vertex_path, KONSTRUCTS_PATH_SIZE);
    shader_path("inventory_fragment.glsl", fragment_path, KONSTRUCTS_PATH_SIZE);

    program = load_program(vertex_path, fragment_path);
    inventory_attrib->program = program;
    inventory_attrib->position = glGetAttribLocation(program, "position");
    inventory_attrib->uv = glGetAttribLocation(program, "uv");
    inventory_attrib->sampler = glGetUniformLocation(program, "sampler");

}

void main_render_text(Player *me, State *s, Player *player, Attrib text_attrib,
        int blocks_recv, int face_count) {

    char text_buffer[KONSTRUCTS_TEXT_MESSAGE_SIZE];
    float ts = 12 * g->scale;
    float tx = ts / 2;
    float ty = g->height - ts;

    if (!is_connected()) {
        snprintf(text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE, "Welcome to Konstructs");
        render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
        ty -= ts * 2;
        snprintf(text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE, "%s", g->text_message);
        render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
        ty -= ts * 2;
    } else {
        int blocks_recv_diff = g->blocks_recv - blocks_recv;
        blocks_recv = g->blocks_recv;
        if (g->debug_screen) {
            int hour = time_of_day() * 24;
            char am_pm = hour < 12 ? 'a' : 'p';
            hour = hour % 12;
            hour = hour ? hour : 12;
            snprintf(
                    text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                    "(%d, %d) (%.2f, %.2f, %.2f) %d%cm %dfps",
                    chunked(s->x), chunked(s->z), s->x, s->y, s->z,
                    hour, am_pm, g->fps.fps);
            render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
            ty -= ts * 2;
            snprintf(
                    text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                    "%d pl, %d ch, %d(%d) bl, %d fa",
                    g->player_count, g->chunk_count, blocks_recv_diff,
                    g->blocks_recv, face_count * 2);
            render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
            ty -= ts * 2;
        } else {
            snprintf(text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                    "Connected to %s as %s; F3: Debug",
                g->server_addr, g->server_user);
            render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
            ty -= ts * 2;
        }
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
        snprintf(text_buffer, KONSTRUCTS_TEXT_MESSAGE_SIZE,
                "%s> %s", g->text_prompt, g->typing_buffer);
        render_text(&text_attrib, ALIGN_LEFT, tx, ty, ts, text_buffer);
        ty -= ts * 2;
    }
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

void main_connect() {
    if (!is_connected()) {
        return;
    }

    char out_hash[41];
    char in_hash[128];

    glfwSetInputMode(g->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(g->window, on_mouse_button);
    glfwSetScrollCallback(g->window, on_scroll);

    client_enable();
    client_connect(g->server_addr, g->server_port);
    client_start();
    g->pending_chunks = 0;
    strcpy(in_hash, g->server_user);
    strcat(in_hash, g->server_pass);
    //hash_password(in_hash, out_hash);
    client_version(PROTOCOL_VERSION, g->server_user, in_hash);

    // Set the chunk window size
    client_chunk(MAX_PENDING_CHUNKS);

    g->typing = 0;
}

void print_usage(char **argv)
{
    printf("USAGE: %s [options]\n", argv[0]);
    printf("OPTIONS: -h/--help                  - Show this help\n");
    printf("         -s/--server   <address>    - Server to enter\n");
    printf("         -u/--username <username>   - Username to login\n");
    printf("         -p/--password <password>   - Passworld to login\n\n");
    exit(0);
}

int init_workers() {
    for (int i = 0; i < WORKERS; i++) {
        Worker *worker = g->workers + i;
        worker->index = i;
        worker->state = WORKER_IDLE;
        //mtx_init(&worker->mtx, mtx_plain);
        //cnd_init(&worker->cnd);
        //thrd_create(&worker->thrd, worker_run, worker);
    }
    return 0;
}

class Konstructs2 : public nanogui::Screen {
public:
    Konstructs2() : nanogui::Screen(Eigen::Vector2i(1024, 768), "Konstructs") {
        using namespace nanogui;

        init_inventory();
        load_textures();
        reset_model();

        if (init_winsock()) {
            printf("Failed to load winsock");
        }

        srand(time(NULL));
        rand();

        // Set global variables
        memset(&g->fps, 0, sizeof(g->fps));
        g->blocks_recv = 0;
        g->debug_screen = 0;
        g->inventory_screen = 0;
        g->chunk_buffer_size = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
        g->chunk_buffer = (char*)malloc(sizeof(char)*g->chunk_buffer_size);
        g->create_radius = CREATE_CHUNK_RADIUS;
        g->render_radius = RENDER_CHUNK_RADIUS;
        g->delete_radius = DELETE_CHUNK_RADIUS;
        g->server_port = DEFAULT_PORT;
        g->server_addr[0] = '\0';
        g->server_user[0] = '\0';
        g->server_pass[0] = '\0';
        g->text_message[0] = '\0';
        g->text_prompt[0] = '\0';
        g->typing_buffer[0] = '\0';
        g->mouse_item = -1;
        move_item.use = 0;
        g->player_count = 1;
        g->window = glfwWindow();
        glfwGetFramebufferSize(g->window, &g->width, &g->height);

        // Set private class variables
        last_update = glfwGetTime();
        connected = 0;
        me = g->players;
        s = &g->players->state;
        previous = glfwGetTime();
        blocks_recv = g->blocks_recv;
        s->y = 250;
        me->id = 0;
        me->name[0] = '\0';
        me->buffer = 0;
        sky_buffer = gen_sky_buffer();
        load_shaders(&block_attrib, &line_attrib, &text_attrib, &sky_attrib, &inventory_attrib);

        // misc
        update_login_prompt();

        std::cout << "Constructor finished" << std::endl;
    }

    ~Konstructs2() {
        client_stop();
        client_disable();
        del_buffer(sky_buffer);
        delete_all_chunks();
        delete_all_players();

        free(g->chunk_buffer);
        free(inventory.items);
#ifdef _WIN32
        WSACleanup();
#endif
        std::cout << "deconstruction finished" << std::endl;
    }

    virtual void drawContents() {
        using namespace nanogui;
        draw_frame();
    }

    int draw_frame() {
        if (is_connected() && !connected) {
            connected = 1;
            main_connect();
        } else if (!is_connected()) {
            g->typing = 1;
        }

        g->scale = get_scale_factor();
        g->fov = 65;

        if (g->time_changed) {
            g->time_changed = 0;
            last_update = glfwGetTime();
            memset(&g->fps, 0, sizeof(g->fps));
        }

        update_fps(&g->fps);
        double now = glfwGetTime();
        double dt = now - previous;
        dt = MIN(dt, 0.2);
        dt = MAX(dt, 0.0);
        previous = now;

        // HANDLE MOUSE INPUT //
        handle_mouse_input();

        // HANDLE MOVEMENT //
        handle_movement(dt);

        if (connected) {
            // HANDLE DATA FROM SERVER //
            Packet packets[MAX_PENDING_CHUNKS / 2];
            float frame_ratio = (float)g->fps.fps / 60.0f;
            // Cubic exponential back off
            float cubed_frame_ratio = frame_ratio * frame_ratio * frame_ratio;
            int received = client_recv(packets, (int)((float)(MAX_PENDING_CHUNKS / 2) * cubed_frame_ratio));

            for (int i = 0; i < received; i++) {
                //parse_buffer(packets[i]);
                free(packets[i].payload);
            }

            // SEND POSITION TO SERVER //
            if (now - last_update > 0.1) {
                last_update = now;
                client_position(s->x, s->y, s->z, s->rx, s->ry);
                std::cout << 1;
            }

            // PREPARE TO RENDER //

            g->observe1 = g->observe1 % g->player_count;
            g->observe2 = g->observe2 % g->player_count;
            delete_chunks();
            del_buffer(me->buffer);
            me->buffer = gen_player_buffer(s->x, s->y, s->z, s->rx, s->ry);
            for (int i = 1; i < g->player_count; i++) {
                interpolate_player(g->players + i);
            }
        }

        Player *player = g->players + g->observe1;

        // RENDER 3-D SCENE //
        //glClear(GL_COLOR_BUFFER_BIT);
        //glClear(GL_DEPTH_BUFFER_BIT);
        //render_sky(&sky_attrib, player, sky_buffer);
        //glClear(GL_DEPTH_BUFFER_BIT);
        int face_count = 0; //render_chunks(&block_attrib, player);
        //render_players(&block_attrib, player);
        //render_wireframe(&line_attrib, player);

        // RENDER HUD //
        if (connected) {
            glClear(GL_DEPTH_BUFFER_BIT);
            if (SHOW_CROSSHAIRS) {
                render_crosshairs(&line_attrib);
            }
        }

        //main_render_text(me, s, player, text_attrib, blocks_recv, face_count);

        if (is_connected()) {
            if(g->inventory_screen) {
                //render_ext_inventory_background(&inventory_attrib);
                //render_ext_inventory_text_blocks(&text_attrib, &block_attrib);
                //render_mouse_block(&block_attrib);
            } else {
                //render_belt_background(&inventory_attrib, inventory.selected);
                //render_belt_text_blocks(&text_attrib, &block_attrib);
                //render_hand_blocks(&block_attrib);
            }
        }
    }

private:
    Attrib block_attrib = {0};
    Attrib line_attrib = {0};
    Attrib text_attrib = {0};
    Attrib sky_attrib = {0};
    Attrib inventory_attrib = {0};
    GLuint sky_buffer;
    double last_update;
    double previous;
    int connected;
    int blocks_recv;
    Player *me;
    State *s;
};

int _main(int argc, char **argv) {
    nanogui::init();

    {
        nanogui::ref<Konstructs2> app = new Konstructs2();
        app->drawAll();
        app->setVisible(true);

        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                    print_usage(argv);
                }
                if (strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) {
                    if (!argv[i+1]) {
                        print_usage(argv);
                    } else {
                        if (check_server(argv[i+1])) {
                            strncpy(g->server_addr, argv[i+1], MAX_ADDR_LENGTH);
                        } else {
                            printf("Failed to resolve '%s', ignoring parameter '%s'\n", argv[i+1], argv[i]);
                        }
                    }
                }
                if (strcmp(argv[i], "--username") == 0 || strcmp(argv[i], "-u") == 0) {
                    if (!argv[i+1]) {
                        print_usage(argv);
                    } else {
                        strncpy(g->server_user, argv[i+1], MAX_NAME_LENGTH);
                    }
                }
                if (strcmp(argv[i], "--password") == 0 || strcmp(argv[i], "-p") == 0) {
                    if (!argv[i+1]) {
                        print_usage(argv);
                    } else {
                        strncpy(g->server_pass, argv[i+1], 64);
                    }
                }
            }
        }

        nanogui::mainloop();
    }

    nanogui::shutdown();
    return 0;
}
