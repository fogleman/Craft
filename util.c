#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

int rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

void update_fps(FPS *fps) {
    fps->frames++;
    double now = glfwGetTime();
    double elapsed = now - fps->since;
    if (elapsed >= 1) {
        int result = fps->frames / elapsed;
        fps->frames = 0;
        fps->since = now;
        // printf("%d\n", result);
    }
}

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

void normalize(float *x, float *y, float *z) {
    float d = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    *x /= d; *y /= d; *z /= d;
}

void mat_identity(float *matrix) {
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

void mat_translate(float *matrix, float dx, float dy, float dz) {
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
    matrix[12] = dx;
    matrix[13] = dy;
    matrix[14] = dz;
    matrix[15] = 1;
}

void mat_rotate(float *matrix, float x, float y, float z, float angle) {
    normalize(&x, &y, &z);
    float s = sinf(angle);
    float c = cosf(angle);
    float m = 1 - c;
    matrix[0] = m * x * x + c;
    matrix[1] = m * x * y - z * s;
    matrix[2] = m * z * x + y * s;
    matrix[3] = 0;
    matrix[4] = m * x * y + z * s;
    matrix[5] = m * y * y + c;
    matrix[6] = m * y * z - x * s;
    matrix[7] = 0;
    matrix[8] = m * z * x - y * s;
    matrix[9] = m * y * z + x * s;
    matrix[10] = m * z * z + c;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void mat_vec_multiply(float *vector, float *a, float *b) {
    float result[4];
    for (int i = 0; i < 4; i++) {
        float total = 0;
        for (int j = 0; j < 4; j++) {
            int p = j * 4 + i;
            int q = j;
            total += a[p] * b[q];
        }
        result[i] = total;
    }
    for (int i = 0; i < 4; i++) {
        vector[i] = result[i];
    }
}

void mat_multiply(float *matrix, float *a, float *b) {
    float result[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            int index = c * 4 + r;
            float total = 0;
            for (int i = 0; i < 4; i++) {
                int p = i * 4 + r;
                int q = c * 4 + i;
                total += a[p] * b[q];
            }
            result[index] = total;
        }
    }
    for (int i = 0; i < 16; i++) {
        matrix[i] = result[i];
    }
}

void mat_frustum(
    float *matrix, float left, float right, float bottom,
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

void mat_perspective(
    float *matrix, float fov, float aspect,
    float znear, float zfar)
{
    float ymax, xmax;
    ymax = znear * tanf(fov * PI / 360.0);
    xmax = ymax * aspect;
    mat_frustum(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void mat_ortho(
    float *matrix,
    float left, float right, float bottom, float top, float near, float far)
{
    matrix[0] = 2 / (right - left);
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 2 / (top - bottom);
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = -2 / (far - near);
    matrix[11] = 0;
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = -(far + near) / (far - near);
    matrix[15] = 1;
}

void make_cube(
    float *vertex, float *normal, float *texture,
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

void make_cube_wireframe(float *vertex, float x, float y, float z, float n) {
    float *v = vertex;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;

    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;

    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
}
