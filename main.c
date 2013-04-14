#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU

#include <GL/glfw.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "noise.h"
#include "util.h"

#define CHUNK_SIZE 32
#define MAX_CHUNKS 1024
#define CREATE_CHUNK_RADIUS 5
#define RENDER_CHUNK_RADIUS 5
#define DELETE_CHUNK_RADIUS 8

const static int FACES[6][3] = {
    { 1, 0, 0},
    {-1, 0, 0},
    { 0, 1, 0},
    { 0,-1, 0},
    { 0, 0, 1},
    { 0, 0,-1}
};

typedef struct {
    Map map;
    int p;
    int q;
    int faces;
    GLuint vertex_buffer;
    GLuint normal_buffer;
    GLuint texture_buffer;
} Chunk;

typedef struct {
    unsigned int frames;
    double timestamp;
} FPS;

void update_fps(FPS *fps) {
    fps->frames++;
    double now = glfwGetTime();
    double elapsed = now - fps->timestamp;
    if (elapsed >= 1) {
        int result = fps->frames / elapsed;
        fps->frames = 0;
        fps->timestamp = now;
        printf("%d\n", result);
    }
}

void update_matrix(float *matrix) {
    int width, height;
    glfwGetWindowSize(&width, &height);
    glViewport(0, 0, width, height);
    perspective_matrix(matrix, 65.0, (float)width / height, 0.1, 128.0);
}

void get_motion_vector(int flying, int sz, int sx, float rx, float ry,
    float *dx, float *dy, float *dz) {
    *dx = 0; *dy = 0; *dz = 0;
    if (!sz && !sx) {
        return;
    }
    float strafe = atan2(sz, sx);
    if (flying) {
        float m = cos(ry);
        float y = sin(ry);
        if (sx) {
            y = 0;
            m = 1;
        }
        if (sz > 0) {
            y = -y;
        }
        *dx = cos(rx + strafe) * m;
        *dy = y;
        *dz = sin(rx + strafe) * m;
    }
    else {
        *dx = cos(rx + strafe);
        *dy = 0;
        *dz = sin(rx + strafe);
    }
}

int collide(Map *map, int height, float *_x, float *_y, float *_z) {
    int result = 0;
    float pad = 0.25;
    float x = *_x;
    float y = *_y;
    float z = *_z;
    int nx = round(x);
    int ny = round(y);
    int nz = round(z);
    float p[3] = {x, y, z};
    int np[3] = {nx, ny, nz};
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 3; i++) {
            int dir = FACES[face][i];
            if (!dir) {
                continue;
            }
            float dist = (p[i] - np[i]) * dir;
            if (dist < pad) {
                continue;
            }
            for (int dy = 0; dy < height; dy++) {
                int op[3] = {nx, ny - dy, nz};
                op[i] += dir;
                if (!map_get(map, op[0], op[1], op[2])) {
                    continue;
                }
                p[i] -= (dist - pad) * dir;
                if (i == 1) {
                    result = 1;
                }
                break;
            }
        }
    }
    *_x = p[0];
    *_y = p[1];
    *_z = p[2];
    return result;
}

void make_world(Map *map, int p, int q) {
    int height = 32;
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            int h = f * (height - 1) + 1;
            int w = 1;
            int t = height * 3 / 8;
            if (h < t) {
                h = t - 1;
                w = 2;
            }
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                w = -1;
            }
            for (int y = 0; y < h; y++) {
                map_set(map, x, y, z, w);
            }
        }
    }
}

void exposed_faces(Map *map, int x, int y, int z,
    int *f1, int *f2, int *f3, int *f4, int *f5, int *f6)
{
    *f1 = map_get(map, x - 1, y, z) == 0;
    *f2 = map_get(map, x + 1, y, z) == 0;
    *f3 = map_get(map, x, y + 1, z) == 0;
    *f4 = map_get(map, x, y - 1, z) == 0 & y > 0;
    *f5 = map_get(map, x, y, z + 1) == 0;
    *f6 = map_get(map, x, y, z - 1) == 0;
}

void make_chunk(Chunk *chunk, int p, int q) {
    Map *map = &chunk->map;
    map_alloc(map);
    make_world(map, p, q);

    int faces = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w < 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        faces += total;
    } END_MAP_FOR_EACH;

    GLfloat *vertex_data = malloc(sizeof(GLfloat) * faces * 18);
    GLfloat *normal_data = malloc(sizeof(GLfloat) * faces * 18);
    GLfloat *texture_data = malloc(sizeof(GLfloat) * faces * 12);
    int vertex_offset = 0;
    int texture_offset = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w < 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        make_cube(
            vertex_data + vertex_offset,
            normal_data + vertex_offset,
            texture_data + texture_offset,
            f1, f2, f3, f4, f5, f6,
            e->x, e->y, e->z, 0.5, e->w);
        vertex_offset += total * 18;
        texture_offset += total * 12;
    } END_MAP_FOR_EACH;

    GLuint vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 18,
        vertex_data
    );
    GLuint normal_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 18,
        normal_data
    );
    GLuint texture_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 12,
        texture_data
    );
    free(vertex_data);
    free(normal_data);
    free(texture_data);

    chunk->p = p;
    chunk->q = q;
    chunk->faces = faces;
    chunk->vertex_buffer = vertex_buffer;
    chunk->normal_buffer = normal_buffer;
    chunk->texture_buffer = texture_buffer;
}

void draw_chunk(
    Chunk *chunk, GLuint vertex_loc, GLuint normal_loc, GLuint uv_loc)
{
    glEnableVertexAttribArray(vertex_loc);
    glEnableVertexAttribArray(normal_loc);
    glEnableVertexAttribArray(uv_loc);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->vertex_buffer);
    glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->normal_buffer);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->texture_buffer);
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, chunk->faces * 9);
}

void ensure_chunks(Chunk *chunks, int *chunk_count, int p, int q, int force) {
    int count = *chunk_count;
    for (int i = 0; i < count; i++) {
        Chunk *chunk = chunks + i;
        int dp = chunk->p - p;
        int dq = chunk->q - q;
        int n = DELETE_CHUNK_RADIUS;
        if (ABS(dp) >= n || ABS(dq) >= n) {
            map_free(&chunk->map);
            glDeleteBuffers(1, &chunk->vertex_buffer);
            glDeleteBuffers(1, &chunk->normal_buffer);
            glDeleteBuffers(1, &chunk->texture_buffer);
            Chunk *other = chunks + (count - 1);
            chunk->map = other->map;
            chunk->p = other->p;
            chunk->q = other->q;
            chunk->faces = other->faces;
            chunk->vertex_buffer = other->vertex_buffer;
            chunk->normal_buffer = other->normal_buffer;
            chunk->texture_buffer = other->texture_buffer;
            count--;
        }
    }
    int n = CREATE_CHUNK_RADIUS;
    for (int i = -n; i <= n; i++) {
        for (int j = -n; j <= n; j++) {
            int a = p + i;
            int b = q + j;
            int create = 1;
            for (int k = 0; k < count; k++) {
                Chunk *chunk = chunks + k;
                if (chunk->p == a && chunk->q == b) {
                    create = 0;
                    break;
                }
            }
            if (create) {
                make_chunk(chunks + count, a, b);
                count++;
                if (!force) {
                    *chunk_count = count;
                    return;
                }
            }
        }
    }
    *chunk_count = count;
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (!glfwOpenWindow(800, 600, 8, 8, 8, 0, 24, 0, GLFW_WINDOW)) {
        return -1;
    }
    glfwSwapInterval(1);
    glfwDisable(GLFW_MOUSE_CURSOR);
    glfwSetWindowTitle("Modern GL");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    Chunk chunks[MAX_CHUNKS];
    int chunk_count = 0;
    ensure_chunks(chunks, &chunk_count, 0, 0, 1);

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glfwLoadTexture2D("texture.tga", 0);

    GLuint program = load_program("vertex.glsl", "fragment.glsl");
    GLuint timer_loc = glGetUniformLocation(program, "timer");
    GLuint matrix_loc = glGetUniformLocation(program, "matrix");
    GLuint rotation_loc = glGetUniformLocation(program, "rotation");
    GLuint center_loc = glGetUniformLocation(program, "center");
    GLuint sampler_loc = glGetUniformLocation(program, "sampler");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint normal_loc = glGetAttribLocation(program, "normal");
    GLuint uv_loc = glGetAttribLocation(program, "uv");

    FPS fps = {0, 0};
    int exclusive = 1;
    float matrix[16];
    float x = 0;
    float y = 32;
    float z = 0;
    float dy = 0;
    float rx = 0;
    float ry = 0;
    int flying = 0;
    int mx, my, px, py;
    glfwGetMousePos(&px, &py);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    double previous = glfwGetTime();

    while (glfwGetWindowParam(GLFW_OPENED)) {
        double now = glfwGetTime();
        double dt = now - previous;
        dt = dt > 0.2 ? 0.2 : dt;
        previous = now;
        update_fps(&fps);
        update_matrix(matrix);

        if (exclusive) {
            glfwGetMousePos(&mx, &my);
            float m = 0.0025;
            rx += (mx - px) * m;
            ry -= (my - py) * m;
            if (rx < 0) rx += RADIANS(360);
            if (rx >= RADIANS(360)) rx -= RADIANS(360);
            ry = MAX(ry, -RADIANS(90));
            ry = MIN(ry, RADIANS(90));
            px = mx;
            py = my;
        }
        if (exclusive && glfwGetKey(GLFW_KEY_ESC)) {
            exclusive = 0;
            glfwEnable(GLFW_MOUSE_CURSOR);
        }
        if (!exclusive && glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
            exclusive = 1;
            glfwDisable(GLFW_MOUSE_CURSOR);
            glfwGetMousePos(&px, &py);
        }

        int sz = 0;
        int sx = 0;
        if (glfwGetKey('Q')) break;
        if (glfwGetKey('W')) sz--;
        if (glfwGetKey('S')) sz++;
        if (glfwGetKey('A')) sx--;
        if (glfwGetKey('D')) sx++;
        if (glfwGetKey('1')) flying = 0;
        if (glfwGetKey('2')) flying = 1;
        if (glfwGetKey(GLFW_KEY_SPACE)) {
            if (dy == 0) {
                dy = 8;
            }
        }
        float vx, vy, vz;
        get_motion_vector(flying, sz, sx, rx, ry, &vx, &vy, &vz);
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
            for (int j = 0; j < chunk_count; j++) {
                if (collide(&chunks[j].map, 2, &x, &y, &z)) {
                    dy = 0;
                }
            }
        }

        glClearColor(0.53, 0.81, 0.92, 1.00);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform1f(timer_loc, glfwGetTime());
        glUniform2f(rotation_loc, rx, ry);
        glUniform3f(center_loc, x, y, z);
        glUniform1i(sampler_loc, 0);

        int p = round(x / CHUNK_SIZE);
        int q = round(z / CHUNK_SIZE);
        ensure_chunks(chunks, &chunk_count, p, q, 0);
        for (int i = 0; i < chunk_count; i++) {
            Chunk *chunk = chunks + i;
            int dp = chunk->p - p;
            int dq = chunk->q - q;
            // if (dp || dq) {
            //     float a1 = atan2(dp, dq) + RADIANS(180);
            //     float a2 = -rx;
            //     float d = ABS(a2 - a1);
            //     if (d > RADIANS(180)) d-= RADIANS(180);
            //     if (d < RADIANS(90)) {
            //         continue;
            //     }
            // }
            int n = RENDER_CHUNK_RADIUS;
            if (ABS(dp) <= n && ABS(dq) <= n) {
                draw_chunk(chunk, position_loc, normal_loc, uv_loc);
            }
        }

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
