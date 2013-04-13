#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU

#include <GL/glfw.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"
#include "modern.h"
#include "noise.h"

typedef struct {
    unsigned int frames;
    double timestamp;
} FPS;

unsigned int randint(unsigned int n) {
    unsigned int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

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

void get_motion_vector(int sz, int sx, float rx, float ry,
    float *dx, float *dy, float *dz) {
    *dx = 0; *dy = 0; *dz = 0;
    if (!sz && !sx) {
        return;
    }
    float strafe = atan2(sz, sx);
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

void make_world(Map *map, int width, int height) {
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < width; z++) {
            float f = simplex2(x * 0.02, z * 0.02, 5, 0.2, 2);
            int h = (f + 1) / 2 * (height - 1) + 1;
            int w = 1;
            if (h < 12) {
                h = 11;
                w = 2;
            }
            for (int y = 0; y < h; y++) {
                map_set(map, x, y, z, w);
            }
        }
    }
}

int exposed_faces(Map *map, int x, int y, int z) {
    int result = 0;
    if (map_get(map, x + 1, y, z) == 0) result++;
    if (map_get(map, x - 1, y, z) == 0) result++;
    if (map_get(map, x, y + 1, z) == 0) result++;
    if (map_get(map, x, y - 1, z) == 0 && y > 0) result++;
    if (map_get(map, x, y, z + 1) == 0) result++;
    if (map_get(map, x, y, z - 1) == 0) result++;
    return result;
}

int main(int argc, char **argv) {
    srand(time(NULL));
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

    Map _map;
    Map *map = &_map;
    map_alloc(map);
    make_world(map, 128, 32);

    int faces = 0;
    MAP_FOR_EACH(map, e) {
        faces += exposed_faces(map, e->x, e->y, e->z);
    } END_MAP_FOR_EACH;

    GLfloat vertex_data[faces * 18];
    GLfloat texture_data[faces * 12];
    int vertex_offset = 0;
    int texture_offset = 0;
    MAP_FOR_EACH(map, e) {
        int left =   map_get(map, e->x - 1, e->y, e->z) == 0;
        int right =  map_get(map, e->x + 1, e->y, e->z) == 0;
        int top =    map_get(map, e->x, e->y + 1, e->z) == 0;
        int bottom = map_get(map, e->x, e->y - 1, e->z) == 0 && e->y > 0;
        int front =  map_get(map, e->x, e->y, e->z + 1) == 0;
        int back =   map_get(map, e->x, e->y, e->z - 1) == 0;
        int total = left + right + top + bottom + front + back;
        if (total == 0) {
            continue;
        }
        make_cube(
            vertex_data + vertex_offset,
            texture_data + texture_offset,
            left, right, top, bottom, front, back,
            e->x, e->y, e->z, 0.5);
        vertex_offset += total * 18;
        texture_offset += total * 12;
    } END_MAP_FOR_EACH;

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(vertex_data),
        vertex_data
    );
    GLuint texture_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(texture_data),
        texture_data
    );

    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glfwLoadTexture2D("texture.tga", 0);

    GLuint program = load_program("vertex.glsl", "fragment.glsl");
    GLuint matrix_loc = glGetUniformLocation(program, "matrix");
    GLuint rotation_loc = glGetUniformLocation(program, "rotation");
    GLuint center_loc = glGetUniformLocation(program, "center");
    GLuint sampler_loc = glGetUniformLocation(program, "sampler");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint uv_loc = glGetAttribLocation(program, "uv");

    FPS fps = {0, 0};
    int exclusive = 1;
    float matrix[16];
    float x = 0;
    float y = 0;
    float z = 0;
    float rx = 0;
    float ry = 0;
    int mx, my, px, py;
    glfwGetMousePos(&px, &py);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    double previous = glfwGetTime();

    while (glfwGetWindowParam(GLFW_OPENED)) {
        double now = glfwGetTime();
        double dt = now - previous;
        previous = now;
        update_fps(&fps);
        update_matrix(matrix);

        if (exclusive) {
            glfwGetMousePos(&mx, &my);
            float m = 0.0025;
            rx += (mx - px) * m;
            ry -= (my - py) * m;
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
        if (glfwGetKey('W')) sz++;
        if (glfwGetKey('S')) sz--;
        if (glfwGetKey('A')) sx++;
        if (glfwGetKey('D')) sx--;
        float dx, dy, dz;
        get_motion_vector(sz, sx, rx, ry, &dx, &dy, &dz);
        float speed = 8;
        x += dx * dt * speed;
        y += dy * dt * speed;
        z += dz * dt * speed;

        glClearColor(0.53, 0.81, 0.92, 1.00);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform2f(rotation_loc, rx, ry);
        glUniform3f(center_loc, x, y, z);
        glUniform1i(sampler_loc, 0);

        glEnableVertexAttribArray(position_loc);
        glEnableVertexAttribArray(uv_loc);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
        glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, faces * 9);

        // for (int dx = -1; dx <= 1; dx++) {
        //     for (int dz = -1; dz <= 1; dz++) {
        //         glUniform3f(center_loc, x + 64 * dx, y, z + 64 * dz);
        //         glDrawArraysInstanced(GL_TRIANGLES, 0, 36, count);
        //     }
        // }

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
