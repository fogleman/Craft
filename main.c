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
#include <time.h>
#include "noise.h"
#include "util.h"

#define VSYNC 1
#define SHOW_FPS 0
#define SIZE 200

static int exclusive = 1;
static int left_click = 0;
static int right_click = 0;
static int flying = 1;
static int ortho = 0;

void update_matrix_2d(float *matrix) {
    int width, height;
    glfwGetWindowSize(&width, &height);
    glViewport(0, 0, width, height);
    mat_ortho(matrix, 0, width, 0, height, -1, 1);
}

void update_matrix_3d(
    float *matrix, float x, float y, float z, float rx, float ry)
{
    float a[16];
    float b[16];
    int width, height;
    glfwGetWindowSize(&width, &height);
    glViewport(0, 0, width, height);
    float aspect = (float)width / height;
    mat_identity(a);
    mat_translate(b, -x, -y, -z);
    mat_multiply(a, b, a);
    mat_rotate(b, cosf(rx), 0, sinf(rx), ry);
    mat_multiply(a, b, a);
    mat_rotate(b, 0, 1, 0, -rx);
    mat_multiply(a, b, a);
    if (ortho) {
        int size = 32;
        mat_ortho(b, -size * aspect, size * aspect, -size, size, -256, 256);
    }
    else {
        mat_perspective(b, 65.0, aspect, 0.1, 1024.0);
    }
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

void make_mesh(GLuint *position_buffer, GLuint *normal_buffer) {
    int width = SIZE;
    int depth = SIZE;
    int size = width * depth * 6 * 3;
    float *position = malloc(sizeof(float) * size);
    float *p = position;
    float *normal = malloc(sizeof(float) * size);
    float *n = normal;
    float amplitude = 32;
    float m = 0.01;
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float x1 = x;
            float x2 = x + 1;
            float z1 = z;
            float z2 = z + 1;
            float y1 = simplex2(x1 * m, z1 * m, 6, 0.5, 2) * amplitude;
            float y2 = simplex2(x2 * m, z1 * m, 6, 0.5, 2) * amplitude;
            float y3 = simplex2(x1 * m, z2 * m, 6, 0.5, 2) * amplitude;
            float y4 = simplex2(x2 * m, z2 * m, 6, 0.5, 2) * amplitude;
            *(p++) = x2; *(p++) = y2; *(p++) = z1;
            *(p++) = x1; *(p++) = y1; *(p++) = z1;
            *(p++) = x1; *(p++) = y3; *(p++) = z2;
            *(p++) = x2; *(p++) = y2; *(p++) = z1;
            *(p++) = x1; *(p++) = y3; *(p++) = z2;
            *(p++) = x2; *(p++) = y4; *(p++) = z2;
            float nx, ny, nz;
            cross(x1 - x2, y3 - y2, z2 - z1, x1 - x2, y1 - y2, z1 - z1,
                &nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            cross(x2 - x2, y4 - y2, z2 - z1, x1 - x2, y3 - y2, z2 - z1,
                &nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
        }
    }
    *position_buffer = make_buffer(
        GL_ARRAY_BUFFER, size * sizeof(float), position
    );
    free(position);
    *normal_buffer = make_buffer(
        GL_ARRAY_BUFFER, size * sizeof(float), normal
    );
    free(normal);
}

void draw_triangles(
    GLuint position_buffer, GLuint normal_buffer,
    GLuint position_loc, GLuint normal_loc, int size, int count)
{
    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(normal_loc);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glVertexAttribPointer(position_loc, size, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glVertexAttribPointer(normal_loc, size, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, count);
    glDisableVertexAttribArray(position_loc);
    glDisableVertexAttribArray(normal_loc);
}

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
    if (key == GLFW_KEY_TAB) {
        flying = !flying;
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
    glfwSwapInterval(VSYNC);
    glfwDisable(GLFW_MOUSE_CURSOR);
    glfwSetWindowTitle("Terrain");
    glfwSetKeyCallback(on_key);
    glfwSetMouseButtonCallback(on_mouse_button);

    #ifndef __APPLE__
        if (glewInit() != GLEW_OK) {
            return -1;
        }
    #endif

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint program = load_program(
        "shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint matrix_loc = glGetUniformLocation(program, "matrix");
    GLuint camera_loc = glGetUniformLocation(program, "camera");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint normal_loc = glGetAttribLocation(program, "normal");

    GLuint position_buffer;
    GLuint normal_buffer;
    make_mesh(&position_buffer, &normal_buffer);

    FPS fps = {0, 0};
    float matrix[16];
    float x = SIZE / 2;
    float z = SIZE / 2;
    float y = 32;
    float dy = 0;
    float rx = 0;
    float ry = -RADIANS(45);
    int px = 0;
    int py = 0;

    glfwGetMousePos(&px, &py);
    double previous = glfwGetTime();
    while (glfwGetWindowParam(GLFW_OPENED)) {
        update_fps(&fps, SHOW_FPS);
        double now = glfwGetTime();
        double dt = MIN(now - previous, 0.2);
        previous = now;

        if (exclusive) {
            int mx, my;
            glfwGetMousePos(&mx, &my);
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
            glfwGetMousePos(&px, &py);
        }

        if (left_click) {
            left_click = 0;
        }

        if (right_click) {
            right_click = 0;
        }

        int sz = 0;
        int sx = 0;
        ortho = glfwGetKey(GLFW_KEY_LSHIFT);
        if (glfwGetKey('Q')) break;
        if (glfwGetKey('W')) sz--;
        if (glfwGetKey('S')) sz++;
        if (glfwGetKey('A')) sx--;
        if (glfwGetKey('D')) sx++;
        float vx, vy, vz;
        get_motion_vector(flying, sz, sx, rx, ry, &vx, &vy, &vz);
        float speed = flying ? 10 : 5;
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
        }

        update_matrix_3d(matrix, x, y, z, rx, ry);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform3f(camera_loc, x, y, z);
        draw_triangles(position_buffer, normal_buffer,
            position_loc, normal_loc, 3, SIZE * SIZE * 6);

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
