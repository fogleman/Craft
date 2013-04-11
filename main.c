#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdio.h>
#include "modern.h"

static GLfloat g_vertex_data[108];

#define N 0.25

static const GLfloat g_uv_data[] = {
    0, 0, N, N, 0, N,
    0, 0, N, 0, N, N,
    N, 0, 0, N, 0, 0,
    N, 0, N, N, 0, N,
    0, N, 0, 0, N, 0,
    0, N, N, 0, N, N,
    0, 0, N, 0, N, N,
    0, 0, N, N, 0, N,
    N, 0, 0, 0, 0, N,
    N, 0, 0, N, N, N,
    0, 0, N, N, N, 0,
    0, 0, 0, N, N, N
};

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
    int width;
    int height;
    glfwGetWindowSize(&width, &height);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    perspective_matrix(matrix, 65.0, (float)width / height, 0.1, 60.0);
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }
    if (!glfwOpenWindow(800, 600, 8, 8, 8, 0, 24, 0, GLFW_WINDOW)) {
        return -1;
    }
    glfwSwapInterval(0);
    glfwSetWindowTitle("Modern GL");

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    make_cube(g_vertex_data, 0, 0, 0, 0.5);
    GLuint vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(g_vertex_data),
        g_vertex_data
    );
    GLuint texture_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(g_uv_data),
        g_uv_data
    );

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glfwLoadTexture2D("texture.tga", 0);

    GLuint program = load_program("vertex.glsl", "fragment.glsl");
    GLuint matrix_loc = glGetUniformLocation(program, "matrix");
    GLuint timer_loc = glGetUniformLocation(program, "timer");
    GLuint sampler_loc = glGetUniformLocation(program, "sampler");
    GLuint position_loc = glGetAttribLocation(program, "position");
    GLuint uv_loc = glGetAttribLocation(program, "uv");

    FPS fps = {0, 0};
    float matrix[16];
    while (glfwGetWindowParam(GLFW_OPENED)) {
        update_fps(&fps);
        update_matrix(matrix);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, matrix);
        glUniform1f(timer_loc, glfwGetTime());
        glUniform1i(sampler_loc, 0);

        glEnableVertexAttribArray(position_loc);
        glEnableVertexAttribArray(uv_loc);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
        glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 81);

        glfwSwapBuffers();
    }
    glfwTerminate();
    return 0;
}
