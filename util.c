#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

char *load_file(const char *path) {
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    char *data = calloc(length + 1, sizeof(char));
    fread(data, 1, length, file);
    fclose(file);
    return data;
}

GLuint make_buffer(GLenum target, GLsizei size, const void *data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, GL_STATIC_DRAW);
    glBindBuffer(target, 0);
    return buffer;
}

GLuint make_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "glCompileShader failed:\n%s\n", info);
        free(info);
    }
    return shader;
}

GLuint load_shader(GLenum type, const char *path) {
    char *data = load_file(path);
    GLuint result = make_shader(type, data);
    free(data);
    return result;
}

GLuint make_program(GLuint shader1, GLuint shader2) {
    GLuint program = glCreateProgram();
    glAttachShader(program, shader1);
    glAttachShader(program, shader2);
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetProgramInfoLog(program, length, NULL, info);
        fprintf(stderr, "glLinkProgram failed: %s\n", info);
        free(info);
    }
    glDetachShader(program, shader1);
    glDetachShader(program, shader2);
    glDeleteShader(shader1);
    glDeleteShader(shader2);
    return program;
}

GLuint load_program(const char *path1, const char *path2) {
    GLuint shader1 = load_shader(GL_VERTEX_SHADER, path1);
    GLuint shader2 = load_shader(GL_FRAGMENT_SHADER, path2);
    GLuint program = make_program(shader1, shader2);
    return program;
}

void identity_matrix(float *matrix) {
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void frustum_matrix(float *matrix, float left, float right, float bottom,
    float top, float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0 * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    matrix[0] = temp / temp2;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;
    matrix[4] = 0.0;
    matrix[5] = temp / temp3;
    matrix[6] = 0.0;
    matrix[7] = 0.0;
    matrix[8] = (right + left) / temp2;
    matrix[9] = (top + bottom) / temp3;
    matrix[10] = (-zfar - znear) / temp4;
    matrix[11] = -1.0;
    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = (-temp * zfar) / temp4;
    matrix[15] = 0.0;
}

void perspective_matrix(float *matrix, float fov, float aspect,
    float znear, float zfar) {
    float ymax, xmax;
    ymax = znear * tanf(fov * PI / 360.0);
    xmax = ymax * aspect;
    frustum_matrix(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void make_cube(float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    float *v = vertex;
    float *d = normal;
    float *t = texture;
    float s = 0.125;
    float a = 0;
    float b = s;
    float du, dv;
    w--;
    if (left) {
        du = w * s; dv = s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (right) {
        du = w * s; dv = s;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
    }
    if (top) {
        du = w * s; dv = s + s;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (bottom) {
        du = w * s; dv = 0;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
    }
    if (front) {
        du = w * s; dv = s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (back) {
        du = w * s; dv = s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
}
