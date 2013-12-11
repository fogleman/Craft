#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"
#include "matrix.h"
#include "util.h"

int rand_int(int n) {
    int result;
    while (n <= (result = rand() / (RAND_MAX / n)));
    return result;
}

double rand_double() {
    return (double)rand() / (double)RAND_MAX;
}

void update_fps(FPS *fps, int show) {
    fps->frames++;
    double now = glfwGetTime();
    double elapsed = now - fps->since;
    if (elapsed >= 1) {
        int result = fps->frames / elapsed;
        fps->frames = 0;
        fps->since = now;
        if (show) {
            printf("%d\n", result);
        }
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

GLuint gen_buffer(GLenum target, GLsizei size, const void *data) {
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

void make_plant(
    float *vertex, float *normal, float *texture,
    float x, float y, float z, float n, int w, float rotation)
{
    float *v = vertex;
    float *d = normal;
    float *t = texture;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du, dv;
    w--;
    du = (w % 16) * s;
    dv = (w / 16 * 3) * s;
    // left
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
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
    // right
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
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
    // front
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z;
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
    // back
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
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
    // rotate the plant
    float mat[16];
    float vec[4] = {0};
    mat_rotate(mat, 0, 1, 0, RADIANS(rotation));
    for (int i = 0; i < 24; i++) {
        // vertex
        v = vertex + i * 3;
        vec[0] = *(v++) - x; vec[1] = *(v++) - y; vec[2] = *(v++) - z;
        mat_vec_multiply(vec, mat, vec);
        v = vertex + i * 3;
        *(v++) = vec[0] + x; *(v++) = vec[1] + y; *(v++) = vec[2] + z;
        // normal
        d = normal + i * 3;
        vec[0] = *(d++); vec[1] = *(d++); vec[2] = *(d++);
        mat_vec_multiply(vec, mat, vec);
        d = normal + i * 3;
        *(d++) = vec[0]; *(d++) = vec[1]; *(d++) = vec[2];
    }
}

void make_cube_faces(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n)
{
    float *v = vertex;
    float *d = normal;
    float *t = texture;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du, dv;
    int w;
    if (left) {
        w = wleft;
        du = (w % 16) * s; dv = (w / 16) * s;
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
        w = wright;
        du = (w % 16) * s; dv = (w / 16) * s;
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
        w = wtop;
        du = (w % 16) * s; dv = (w / 16) * s;
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
        w = wbottom;
        du = (w % 16) * s; dv = (w / 16) * s;
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
        w = wfront;
        du = (w % 16) * s; dv = (w / 16) * s;
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
        w = wback;
        du = (w % 16) * s; dv = (w / 16) * s;
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

void make_cube(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    int wleft, wright, wtop, wbottom, wfront, wback;
    w--;
    wbottom = w;
    wleft = wright = wfront = wback = w + 16;
    wtop = w + 32;
    make_cube_faces(
        vertex, normal, texture,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        x, y, z, n);
}

void make_player(
    float *vertex, float *normal, float *texture,
    float x, float y, float z, float rx, float ry)
{
    make_cube_faces(
        vertex, normal, texture,
        1, 1, 1, 1, 1, 1,
        13, 13, 13 + 32, 13, 13, 13 + 16,
        0, 0, 0, 0.4);
    float a[16];
    float b[16];
    float vec[4] = {0};
    vec[3] = 1;
    mat_identity(a);
    mat_rotate(b, 0, 1, 0, rx);
    mat_multiply(a, b, a);
    mat_rotate(b, cosf(rx), 0, sinf(rx), -ry);
    mat_multiply(a, b, a);
    mat_translate(b, x, y, z);
    mat_multiply(a, b, a);
    for (int i = 0; i < 36; i++) {
        // vertex
        float *v = vertex + i * 3;
        vec[0] = *(v++); vec[1] = *(v++); vec[2] = *(v++);
        mat_vec_multiply(vec, a, vec);
        v = vertex + i * 3;
        *(v++) = vec[0]; *(v++) = vec[1]; *(v++) = vec[2];
        // normal
        float *d = normal + i * 3;
        vec[0] = *(d++); vec[1] = *(d++); vec[2] = *(d++);
        mat_vec_multiply(vec, a, vec);
        d = normal + i * 3;
        *(d++) = vec[0]; *(d++) = vec[1]; *(d++) = vec[2];
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

void load_png_texture(const char *file_name) {
    unsigned error;
    unsigned char *image;
    unsigned width, height;
    error = lodepng_decode32_file(&image, &width, &height, file_name);
    if (error) {
        fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, image);
    free(image);
}
