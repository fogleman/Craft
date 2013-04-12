#include <GL/glew.h>
#include <GL/glfw.h>
#include <math.h>
#include <stdio.h>
#include "modern.h"

typedef struct {
    GLint x;
    GLint y;
    GLint z;
    GLint w;
} Block;

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
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    perspective_matrix(matrix, 65.0, (float)width / height, 0.1, 60.0);
}

void get_motion_vector(int sz, int sx, float rx, float ry,
    float *dx, float *dy, float *dz) {
    *dx = 0; *dy = 0; *dz = 0;
    if (!sz && !sx) {
        return;
    }
    float strafe = atan2(sz, sx);
    float m = cos(ry);
    *dy = sin(ry);
    if (sx) {
        *dy = 0;
        m = 1;
    }
    if (sz > 0) {
        *dy *= -1;
    }
    *dx = cos(rx + strafe) * m;
    *dz = sin(rx + strafe) * m;
}

void make_world(Block *world) {
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 32; x++) {
            for (int z = 0; z < 32; z++) {
                world->x = x;
                world->y = y;
                world->z = z;
                world->w = 2;
                world++;
            }
        }
    }
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }
    if (!glfwOpenWindow(800, 600, 8, 8, 8, 0, 24, 0, GLFW_WINDOW)) {
        return -1;
    }
    glfwSwapInterval(1);
    glfwDisable(GLFW_MOUSE_CURSOR);
    glfwSetWindowTitle("Modern GL");

    int count = 65536;
    GLfloat vertex_data[108];
    GLfloat texture_data[72];
    Block world_data[count];
    make_cube(vertex_data, texture_data, 0, 0, 0, 0.5);
    make_world(world_data);

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
    GLuint world_buffer = make_buffer(
        GL_TEXTURE_BUFFER,
        sizeof(world_data),
        world_data
    );

    GLuint world_texture;
    glGenTextures(1, &world_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, world_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32I, world_buffer);

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
    GLuint rotation_loc = glGetUniformLocation(program, "rotation");
    GLuint center_loc = glGetUniformLocation(program, "center");
    GLuint sampler_loc = glGetUniformLocation(program, "sampler");
    GLuint world_loc = glGetUniformLocation(program, "world");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint uv_loc = glGetAttribLocation(program, "uv");

    FPS fps = {0, 0};
    float matrix[16];
    float x = 0;
    float y = 0;
    float z = 0;
    float rx = 0;
    float ry = 0;
    int mx, my, px, py;
    glfwGetMousePos(&px, &py);
    glEnable(GL_CULL_FACE);
    double previous = glfwGetTime();
    while (glfwGetWindowParam(GLFW_OPENED)) {
        double now = glfwGetTime();
        double dt = now - previous;
        previous = now;
        update_fps(&fps);
        update_matrix(matrix);

        glfwGetMousePos(&mx, &my);
        float m = 0.0025;
        float t = RADIANS(90);
        rx += (mx - px) * m;
        ry -= (my - py) * m;
        ry = ry < -t ? -t : ry;
        ry = ry > t ? t : ry;
        px = mx;
        py = my;

        int sz = 0;
        int sx = 0;
        if (glfwGetKey('W')) sz++;
        if (glfwGetKey('S')) sz--;
        if (glfwGetKey('A')) sx++;
        if (glfwGetKey('D')) sx--;
        float dx, dy, dz;
        get_motion_vector(sz, sx, rx, ry, &dx, &dy, &dz);
        float speed = 4;
        x += dx * dt * speed;
        y += dy * dt * speed;
        z += dz * dt * speed;

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform1f(timer_loc, now);
        glUniform2f(rotation_loc, rx, ry);
        glUniform3f(center_loc, x, y, z);
        glUniform1i(sampler_loc, 0);
        glUniform1i(world_loc, 1);

        glEnableVertexAttribArray(position_loc);
        glEnableVertexAttribArray(uv_loc);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
        glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, count);

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
