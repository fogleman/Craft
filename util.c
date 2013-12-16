#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    char *data = calloc(length + 1, sizeof(char));
    fread(data, 1, length, file);
    fclose(file);
    return data;
}

void malloc_buffers(
    int components, int faces,
    GLfloat **position_data, GLfloat **normal_data, GLfloat **uv_data)
{
    if (position_data) {
        *position_data = malloc(sizeof(GLfloat) * faces * 6 * components);
    }
    if (normal_data) {
        *normal_data = malloc(sizeof(GLfloat) * faces * 6 * components);
    }
    if (uv_data) {
        *uv_data = malloc(sizeof(GLfloat) * faces * 6 * 2);
    }
}

GLuint gen_buffer(GLenum target, GLsizei size, const void *data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, size, data, GL_STATIC_DRAW);
    glBindBuffer(target, 0);
    return buffer;
}

void gen_buffers(
    int components, int faces,
    GLfloat *position_data, GLfloat *normal_data, GLfloat *uv_data,
    GLuint *position_buffer, GLuint *normal_buffer, GLuint *uv_buffer)
{
    if (position_buffer) {
        glDeleteBuffers(1, position_buffer);
        *position_buffer = gen_buffer(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * faces * 6 * components,
            position_data
        );
        free(position_data);
    }
    if (normal_buffer) {
        glDeleteBuffers(1, normal_buffer);
        *normal_buffer = gen_buffer(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * faces * 6 * components,
            normal_data
        );
        free(normal_data);
    }
    if (uv_buffer) {
        glDeleteBuffers(1, uv_buffer);
        *uv_buffer = gen_buffer(
            GL_ARRAY_BUFFER,
            sizeof(GLfloat) * faces * 6 * 2,
            uv_data
        );
        free(uv_data);
    }
}

GLuint make_shader(GLenum type, const char *source) {
    assert(source);
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
    assert(path);
    char *data = load_file(path);
    if (!data) return 0;
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

void load_png_texture(const char *file_name) {
    unsigned error;
    unsigned char *image;
    unsigned width, height;
    error = lodepng_decode32_file(&image, &width, &height, file_name);
    if (error) {
        fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, image);
    free(image);
}
