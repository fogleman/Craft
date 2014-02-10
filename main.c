#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#else
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "noise.h"
#include "util.h"

#define FULLSCREEN 0
#define VSYNC 1
#define SHOW_FPS 0
#define SIZE 400

static GLFWwindow *window;
static int exclusive = 1;
static int left_click = 0;
static int right_click = 0;
static int flying = 0;
static int ortho = 0;
static float fov = 65.0;
static float grid[SIZE * SIZE];

void update_matrix_2d(float *matrix) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    mat_ortho(matrix, 0, width, 0, height, -1, 1);
}

void update_matrix_3d(
    float *matrix, float x, float y, float z, float rx, float ry)
{
    float a[16];
    float b[16];
    int width, height;
    glfwGetWindowSize(window, &width, &height);
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
        int size = 128;
        mat_ortho(b, -size * aspect, size * aspect, -size, size, -256, 256);
    }
    else {
        mat_perspective(b, fov, aspect, 0.1, 1024.0);
    }
    mat_multiply(a, b, a);
    mat_identity(matrix);
    mat_multiply(matrix, a, matrix);
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
    int count = width * depth * 6 * 3;
    float *position = malloc(sizeof(float) * count);
    float *p = position;
    float *normal = malloc(sizeof(float) * count);
    float *n = normal;
    float amplitude = 64;
    float persistence = 0.5;
    float m = 0.005;
    float lookup[width + 1][depth + 1][3];
    memset(lookup, 0, (width + 1) * (depth + 1) * 3 * sizeof(float));
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float x1 = x;
            float x2 = x + 1;
            float z1 = z;
            float z2 = z + 1;
            float y1 = simplex2(x1 * m, z1 * m, 4, persistence, 2) * amplitude;
            float y2 = simplex2(x2 * m, z1 * m, 4, persistence, 2) * amplitude;
            float y3 = simplex2(x1 * m, z2 * m, 4, persistence, 2) * amplitude;
            float y4 = simplex2(x2 * m, z2 * m, 4, persistence, 2) * amplitude;
            float y = (y1 + y2 + y3 + y4) / 4;
            grid[z * SIZE + x] = y;
            *(p++) = x2; *(p++) = y2; *(p++) = z1;
            *(p++) = x1; *(p++) = y1; *(p++) = z1;
            *(p++) = x1; *(p++) = y3; *(p++) = z2;
            *(p++) = x2; *(p++) = y2; *(p++) = z1;
            *(p++) = x1; *(p++) = y3; *(p++) = z2;
            *(p++) = x2; *(p++) = y4; *(p++) = z2;
            float nx, ny, nz;
            cross(x1 - x2, y3 - y2, z2 - z1, x1 - x2, y1 - y2, z1 - z1,
                &nx, &ny, &nz);
            lookup[x + 1][z + 0][0] += nx; lookup[x + 1][z + 0][1] += ny; lookup[x + 1][z + 0][2] += nz;
            lookup[x + 0][z + 0][0] += nx; lookup[x + 0][z + 0][1] += ny; lookup[x + 0][z + 0][2] += nz;
            lookup[x + 0][z + 1][0] += nx; lookup[x + 0][z + 1][1] += ny; lookup[x + 0][z + 1][2] += nz;
            cross(x2 - x2, y4 - y2, z2 - z1, x1 - x2, y3 - y2, z2 - z1,
                &nx, &ny, &nz);
            lookup[x + 1][z + 0][0] += nx; lookup[x + 1][z + 0][1] += ny; lookup[x + 1][z + 0][2] += nz;
            lookup[x + 0][z + 1][0] += nx; lookup[x + 0][z + 1][1] += ny; lookup[x + 0][z + 1][2] += nz;
            lookup[x + 1][z + 1][0] += nx; lookup[x + 1][z + 1][1] += ny; lookup[x + 1][z + 1][2] += nz;
        }
    }
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float nx, ny, nz;
            nx = lookup[x + 1][z + 0][0]; ny = lookup[x + 1][z + 0][1]; nz = lookup[x + 1][z + 0][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            nx = lookup[x + 0][z + 0][0]; ny = lookup[x + 0][z + 0][1]; nz = lookup[x + 0][z + 0][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            nx = lookup[x + 0][z + 1][0]; ny = lookup[x + 0][z + 1][1]; nz = lookup[x + 0][z + 1][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            nx = lookup[x + 1][z + 0][0]; ny = lookup[x + 1][z + 0][1]; nz = lookup[x + 1][z + 0][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            nx = lookup[x + 0][z + 1][0]; ny = lookup[x + 0][z + 1][1]; nz = lookup[x + 0][z + 1][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
            nx = lookup[x + 1][z + 1][0]; ny = lookup[x + 1][z + 1][1]; nz = lookup[x + 1][z + 1][2];
            normalize(&nx, &ny, &nz);
            *(n++) = nx; *(n++) = ny; *(n++) = nz;
        }
    }
    *position_buffer = make_buffer(
        GL_ARRAY_BUFFER, count * sizeof(float), position
    );
    free(position);
    *normal_buffer = make_buffer(
        GL_ARRAY_BUFFER, count * sizeof(float), normal
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

void on_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        if (exclusive) {
            exclusive = 0;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (key == GLFW_KEY_TAB) {
        flying = !flying;
    }
}

void on_mouse_button(GLFWwindow *window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    if (button == 0) {
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
    if (button == 1) {
        if (exclusive) {
            right_click = 1;
        }
    }
}

void create_window() {
    #ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #endif
    int width = 1024;
    int height = 768;
    GLFWmonitor *monitor = NULL;
    if (FULLSCREEN) {
        int mode_count;
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *modes = glfwGetVideoModes(monitor, &mode_count);
        width = modes[mode_count - 1].width;
        height = modes[mode_count - 1].height;
    }
    window = glfwCreateWindow(width, height, "Craft", monitor, NULL);
}

int main(int argc, char **argv) {
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
    glfwSetMouseButtonCallback(window, on_mouse_button);

    #ifndef __APPLE__
        if (glewInit() != GLEW_OK) {
            return -1;
        }
    #endif

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.53, 0.81, 0.92, 1);

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
    double px = 0;
    double py = 0;

    glfwGetCursorPos(window, &px, &py);
    double previous = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        update_fps(&fps, SHOW_FPS);
        double now = glfwGetTime();
        double dt = MIN(now - previous, 0.2);
        previous = now;

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

        if (left_click) {
            left_click = 0;
        }

        if (right_click) {
            right_click = 0;
        }

        int sz = 0;
        int sx = 0;
        ortho = glfwGetKey(window, 'F');
        fov = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? 15.0 : 65.0;
        if (glfwGetKey(window, 'Q')) break;
        if (glfwGetKey(window, 'W')) sz--;
        if (glfwGetKey(window, 'S')) sz++;
        if (glfwGetKey(window, 'A')) sx--;
        if (glfwGetKey(window, 'D')) sx++;
        if (dy == 0 && glfwGetKey(window, GLFW_KEY_SPACE)) {
            dy = 8;
        }
        float vx, vy, vz;
        get_motion_vector(flying, sz, sx, rx, ry, &vx, &vy, &vz);
        if (glfwGetKey(window, 'Z')) {
            vx = -1; vy = 0; vz = 0;
        }
        if (glfwGetKey(window, 'X')) {
            vx = 1; vy = 0; vz = 0;
        }
        if (glfwGetKey(window, 'C')) {
            vx = 0; vy = -1; vz = 0;
        }
        if (glfwGetKey(window, 'V')) {
            vx = 0; vy = 1; vz = 0;
        }
        if (glfwGetKey(window, 'B')) {
            vx = 0; vy = 0; vz = -1;
        }
        if (glfwGetKey(window, 'N')) {
            vx = 0; vy = 0; vz = 1;
        }
        float speed = flying ? 20 : 50;
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

        x = MAX(x, 0);
        z = MAX(z, 0);
        x = MIN(x, SIZE - 1);
        z = MIN(z, SIZE - 1);
        int nx = x;
        int nz = z;
        float x0 = x - nx;
        float z0 = z - nz;
        float f00 = grid[(nz + 0) * SIZE + (nx + 0)];
        float f01 = grid[(nz + 1) * SIZE + (nx + 0)];
        float f10 = grid[(nz + 0) * SIZE + (nx + 1)];
        float f11 = grid[(nz + 1) * SIZE + (nx + 1)];
        float ty =
            f00 + (f10 - f00) * x0 + (f01 - f00) * z0 +
            (f00 - f10 - f01 + f11) * x0 * z0;
        y = MAX(y, ty + 5);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update_matrix_3d(matrix, x, y, z, rx, ry);

        // render chunks
        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform3f(camera_loc, x, y, z);
        draw_triangles(position_buffer, normal_buffer,
            position_loc, normal_loc, 3, SIZE * SIZE * 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
