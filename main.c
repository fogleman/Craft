#include <GL/glew.h>
#include <GL/glfw.h>
#include "modern.h"

static GLfloat g_vertex_buffer_data[72];

static const GLushort g_element_buffer_data[] = {
    0, 3, 2,
    0, 1, 3,
    4, 7, 5,
    4, 6, 7,
    2, 3, 7,
    2, 7, 6,
    0, 4, 5,
    0, 5, 1,
    1, 5, 7,
    1, 7, 3,
    0, 6, 4,
    0, 2, 6,
};

void set_3d(float *matrix) {
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
    glfwSetWindowTitle("Modern GL");
    if (glewInit() != GLEW_OK) {
        return -1;
    }
    make_cube(g_vertex_buffer_data, 0, 0, -10, 0.5);
    make_cube(g_vertex_buffer_data + 24, -3, 0, -10, 0.5);
    make_cube(g_vertex_buffer_data + 48, 3, 0, -10, 0.5);
    GLuint vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        sizeof(g_vertex_buffer_data),
        g_vertex_buffer_data
    );
    GLuint element_buffer = make_buffer(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(g_element_buffer_data),
        g_element_buffer_data
    );
    GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, "vertex.sl");
    GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, "fragment.sl");
    GLuint program = make_program(vertex_shader, fragment_shader);
    float matrix[16];
    while (glfwGetWindowParam(GLFW_OPENED)) {
        glClearColor(0.5, 0.69, 1.0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        set_3d(matrix);
        glUniformMatrix4fv(
            glGetUniformLocation(program, "matrix"),
            1, GL_FALSE, matrix);
        glUniform1f(
            glGetUniformLocation(program, "timer"),
            glfwGetTime());
        glUseProgram(program);

        GLuint index = glGetAttribLocation(program, "position");
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, 0);
        glEnableVertexAttribArray(index);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
        glDrawElementsBaseVertex(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0, 0);
        glDrawElementsBaseVertex(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0, 8);
        glDrawElementsBaseVertex(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0, 16);
        glDisableVertexAttribArray(index);

        glfwSwapBuffers();
    }
    return 0;
}
