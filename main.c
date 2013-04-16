#ifdef __APPLE__
    #define GLFW_INCLUDE_GL3
    #define GLFW_NO_GLU
#else
    #include <GL/glew.h>
#endif

#include <GL/glfw.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "noise.h"
#include "util.h"

#define CHUNK_SIZE 32
#define MAX_CHUNKS 1024
#define CREATE_CHUNK_RADIUS 7
#define RENDER_CHUNK_RADIUS 7
#define DELETE_CHUNK_RADIUS 12

typedef struct {
    Map map;
    int p;
    int q;
    int faces;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint uv_buffer;
} Chunk;

void update_matrix(float *matrix,
    float x, float y, float z, float rx, float ry)
{
    int width, height;
    float a[16];
    float b[16];
    glfwGetWindowSize(&width, &height);
    glViewport(0, 0, width, height);
    mat_identity(a);
    mat_translate(b, -x, -y, -z);
    mat_multiply(a, b, a);
    mat_rotate(b, cosf(rx), 0, sinf(rx), ry);
    mat_multiply(a, b, a);
    mat_rotate(b, 0, 1, 0, -rx);
    mat_multiply(a, b, a);
    mat_perspective(b, 65.0, (float)width / height, 0.1, 1024.0);
    mat_multiply(a, b, a);
    for (int i = 0; i < 16; i++) {
        matrix[i] = a[i];
    }
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

int chunk_distance(Chunk *chunk, int p, int q) {
    int dp = ABS(chunk->p - p);
    int dq = ABS(chunk->q - q);
    return MIN(dp, dq);
}

int _hit_test(Map *map,
    float max_distance, int previous,
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
            if (map_get(map, nx, ny, nz)) {
                if (previous) {
                    *hx = px; *hy = py; *hz = pz;
                }
                else {
                    *hx = nx; *hy = ny; *hz = nz;
                }
                return 1;
            }
            px = nx; py = ny; pz = nz;
        }
        x += vx / m; y += vy / m; z += vz / m;
    }
    return 0;
}

int hit_test(Chunk *chunks, int chunk_count, int previous,
    float x, float y, float z, float rx, float ry,
    int *bx, int *by, int *bz)
{
    int result = 0;
    float best = 0;
    int p = floorf(roundf(x) / CHUNK_SIZE);
    int q = floorf(roundf(z) / CHUNK_SIZE);
    float vx, vy, vz;
    get_sight_vector(rx, ry, &vx, &vy, &vz);
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > 1) {
            continue;
        }
        int hx, hy, hz;
        if (_hit_test(&chunk->map, 4, previous,
            x, y, z, vx, vy, vz, &hx, &hy, &hz))
        {
            float d = sqrtf(
                powf(hx - x, 2) + powf(hy - y, 2) + powf(hz - z, 2));
            if (best == 0 || d < best) {
                best = d;
                *bx = hx; *by = hy; *bz = hz;
            }
            result = 1;
        }
    }
    return result;
}

int _collide(Map *map, int height, float *x, float *y, float *z) {
    int result = 0;
    int nx = roundf(*x);
    int ny = roundf(*y);
    int nz = roundf(*z);
    float px = *x - nx;
    float py = *y - ny;
    float pz = *z - nz;
    float pad = 0.25;
    for (int dy = 0; dy < height; dy++) {
        if (px < -pad && map_get(map, nx - 1, ny - dy, nz)) {
            *x = nx - pad;
        }
        if (px > pad && map_get(map, nx + 1, ny - dy, nz)) {
            *x = nx + pad;
        }
        if (py < -pad && map_get(map, nx, ny - dy - 1, nz)) {
            *y = ny - pad;
            result = 1;
        }
        if (py > pad && map_get(map, nx, ny - dy + 1, nz)) {
            *y = ny + pad;
            result = 1;
        }
        if (pz < -pad && map_get(map, nx, ny - dy, nz - 1)) {
            *z = nz - pad;
        }
        if (pz > pad && map_get(map, nx, ny - dy, nz + 1)) {
            *z = nz + pad;
        }
    }
    return result;
}

int collide(Chunk *chunks, int chunk_count, float *x, float *y, float *z) {
    int result = 0;
    int p = floorf(roundf(*x) / CHUNK_SIZE);
    int q = floorf(roundf(*z) / CHUNK_SIZE);
    for (int i = 0; i < chunk_count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) > 1) {
            continue;
        }
        if (_collide(&chunk->map, 2, x, y, z)) {
            result = 1;
        }
    }
    return result;
}

void make_world(Map *map, int p, int q) {
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = simplex2(x * 0.01, z * 0.01, 2, 0.9, 2);
            int mh = g * 32 + 16;
            int h = f * mh;
            int w = 1;
            int t = 12;
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

void update_chunk(Chunk *chunk) {
    Map *map = &chunk->map;

    if (chunk->faces) {
        glDeleteBuffers(1, &chunk->position_buffer);
        glDeleteBuffers(1, &chunk->normal_buffer);
        glDeleteBuffers(1, &chunk->uv_buffer);
    }

    int faces = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        faces += total;
    } END_MAP_FOR_EACH;

    GLfloat *position_data = malloc(sizeof(GLfloat) * faces * 18);
    GLfloat *normal_data = malloc(sizeof(GLfloat) * faces * 18);
    GLfloat *uv_data = malloc(sizeof(GLfloat) * faces * 12);
    int position_offset = 0;
    int uv_offset = 0;
    MAP_FOR_EACH(map, e) {
        if (e->w <= 0) {
            continue;
        }
        int f1, f2, f3, f4, f5, f6;
        exposed_faces(map, e->x, e->y, e->z, &f1, &f2, &f3, &f4, &f5, &f6);
        int total = f1 + f2 + f3 + f4 + f5 + f6;
        if (total == 0) {
            continue;
        }
        make_cube(
            position_data + position_offset,
            normal_data + position_offset,
            uv_data + uv_offset,
            f1, f2, f3, f4, f5, f6,
            e->x, e->y, e->z, 0.5, e->w);
        position_offset += total * 18;
        uv_offset += total * 12;
    } END_MAP_FOR_EACH;

    GLuint position_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 18,
        position_data
    );
    GLuint normal_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 18,
        normal_data
    );
    GLuint uv_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(GLfloat) * faces * 12,
        uv_data
    );
    free(position_data);
    free(normal_data);
    free(uv_data);

    chunk->faces = faces;
    chunk->position_buffer = position_buffer;
    chunk->normal_buffer = normal_buffer;
    chunk->uv_buffer = uv_buffer;
}

void make_chunk(Chunk *chunk, int p, int q) {
    chunk->p = p;
    chunk->q = q;
    chunk->faces = 0;
    Map *map = &chunk->map;
    map_alloc(map);
    make_world(map, p, q);
    update_chunk(chunk);
}

void draw_chunk(
    Chunk *chunk, GLuint position_loc, GLuint normal_loc, GLuint uv_loc)
{
    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(normal_loc);
    glEnableVertexAttribArray(uv_loc);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->position_buffer);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->normal_buffer);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->uv_buffer);
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, chunk->faces * 9);
}

void ensure_chunks(Chunk *chunks, int *chunk_count, int p, int q, int force) {
    int count = *chunk_count;
    for (int i = 0; i < count; i++) {
        Chunk *chunk = chunks + i;
        if (chunk_distance(chunk, p, q) >= DELETE_CHUNK_RADIUS) {
            map_free(&chunk->map);
            glDeleteBuffers(1, &chunk->position_buffer);
            glDeleteBuffers(1, &chunk->normal_buffer);
            glDeleteBuffers(1, &chunk->uv_buffer);
            Chunk *other = chunks + (count - 1);
            chunk->map = other->map;
            chunk->p = other->p;
            chunk->q = other->q;
            chunk->faces = other->faces;
            chunk->position_buffer = other->position_buffer;
            chunk->normal_buffer = other->normal_buffer;
            chunk->uv_buffer = other->uv_buffer;
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

static int exclusive = 1;
static int left_click = 0;
static int right_click = 0;

void on_key(int key, int pressed) {
    if (!pressed) {
        return;
    }
    if (key == GLFW_KEY_ESC) {
        if (exclusive) {
            exclusive = 0;
            glfwEnable(GLFW_MOUSE_CURSOR);
        }
    }
}

void on_mouse_button(int button, int pressed) {
    if (!pressed) {
        return;
    }
    if (button == 0) {
        if (exclusive) {
            left_click = 1;
        }
        else {
            exclusive = 1;
            glfwDisable(GLFW_MOUSE_CURSOR);
        }
    }
    if (button == 1) {
        if (exclusive) {
            right_click = 1;
        }
    }
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }
    #ifdef __APPLE__
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
        glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #endif
    if (!glfwOpenWindow(800, 600, 8, 8, 8, 0, 24, 0, GLFW_WINDOW)) {
        return -1;
    }
    glfwSwapInterval(1);
    glfwDisable(GLFW_MOUSE_CURSOR);
    glfwSetWindowTitle("Modern GL");
    glfwSetKeyCallback(on_key);
    glfwSetMouseButtonCallback(on_mouse_button);

    #ifndef __APPLE__
        if (glewInit() != GLEW_OK) {
            return -1;
        }
    #endif

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
    GLuint matrix_loc = glGetUniformLocation(program, "matrix");
    GLuint timer_loc = glGetUniformLocation(program, "timer");
    GLuint sampler_loc = glGetUniformLocation(program, "sampler");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint normal_loc = glGetAttribLocation(program, "normal");
    GLuint uv_loc = glGetAttribLocation(program, "uv");

    FPS fps = {0, 0};
    float matrix[16];
    float x = 0;
    float y = 32;
    float z = 0;
    float dy = 0;
    float rx = 0;
    float ry = 0;
    int flying = 0;
    int px, py;
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

        if (exclusive) {
            int mx, my;
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
        if (left_click) {
            left_click = 0;
            int hx, hy, hz;
            if (hit_test(chunks, chunk_count, 0, x, y, z, rx, ry,
                &hx, &hy, &hz))
            {
                for (int i = 0; i < chunk_count; i++) {
                    Chunk *chunk = chunks + i;
                    Map *map = &chunk->map;
                    if (map_get(map, hx, hy, hz)) {
                        map_set(map, hx, hy, hz, 0);
                        update_chunk(chunk);
                    }
                }
            }
        }
        if (right_click) {
            right_click = 0;
            int hx, hy, hz;
            if (hit_test(chunks, chunk_count, 1, x, y, z, rx, ry,
                &hx, &hy, &hz))
            {
                int p = floorf((float)hx / CHUNK_SIZE);
                int q = floorf((float)hz / CHUNK_SIZE);
                for (int i = 0; i < chunk_count; i++) {
                    Chunk *chunk = chunks + i;
                    if (chunk->p == p && chunk->q == q) {
                        Map *map = &chunk->map;
                        map_set(map, hx, hy, hz, 1);
                        update_chunk(chunk);
                        break;
                    }
                }
            }
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
            if (collide(chunks, chunk_count, &x, &y, &z)) {
                dy = 0;
            }
        }

        int p = floorf(roundf(x) / CHUNK_SIZE);
        int q = floorf(roundf(z) / CHUNK_SIZE);
        update_matrix(matrix, x, y, z, rx, ry);

        glClearColor(0.53, 0.81, 0.92, 1.00);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform1f(timer_loc, glfwGetTime());
        glUniform1i(sampler_loc, 0);

        ensure_chunks(chunks, &chunk_count, p, q, 0);
        int rendered_chunks = 0;
        int rendered_faces = 0;
        for (int i = 0; i < chunk_count; i++) {
            Chunk *chunk = chunks + i;
            if (chunk_distance(chunk, p, q) > RENDER_CHUNK_RADIUS) {
                continue;
            }
            int visible = 0;
            for (int dp = 0; dp <= 1; dp++) {
                for (int dq = 0; dq <= 1; dq++) {
                    for (int my = 0; my <= 1; my++) {
                        float vec[4] = {
                            (chunk->p + dp) * CHUNK_SIZE,
                            y * my,
                            (chunk->q + dq) * CHUNK_SIZE,
                            1};
                        mat_vec_multiply(vec, matrix, vec);
                        if (vec[3] >= 0) {
                            visible = 1;
                        }
                    }
                }
            }
            if (!visible) {
                continue;
            }
            draw_chunk(chunk, position_loc, normal_loc, uv_loc);
            rendered_chunks += 1;
            rendered_faces += chunk->faces;
        }
        // printf("%d chunks, %d faces\n", rendered_chunks, rendered_faces);

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
