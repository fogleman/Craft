#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <lodepng.h>
#include "matrix.h"
#include "util.h"

void update_fps(FPS *fps) {
    fps->frames++;
    double now = glfwGetTime();
    double elapsed = now - fps->since;
    if (elapsed >= 1) {
        fps->old_fps = fps->fps;
        fps->fps = round(fps->frames / elapsed);
        fps->frames = 0;
        fps->since = now;
    }
}

char *load_file(const char *path) {
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    char *data = (char*)calloc(length + 1, sizeof(char));
    int read = fread(data, 1, length, file);
    fclose(file);
    if(read != length) {
      fprintf(stderr, "Failed to load file %s\n", path);
      exit(1);
    }
    return data;
}

GLuint gen_buffer(GLsizei size, GLfloat *data) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return buffer;
}

GLuint gen_faces(int components, int faces, GLfloat *data) {
    GLuint buffer = gen_buffer(
        sizeof(GLfloat) * 6 * components * faces, data);
    free(data);
    return buffer;
}

void flip_image_vertical(
    unsigned char *data, unsigned int width, unsigned int height)
{
    unsigned int size = width * height * 4;
    unsigned int stride = sizeof(char) * width * 4;
    unsigned char *new_data = (unsigned char*)malloc(sizeof(unsigned char) * size);
    for (unsigned int i = 0; i < height; i++) {
        unsigned int j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }
    memcpy(data, new_data, size);
    free(new_data);
}

void load_png_texture(const char *file_name) {
    unsigned int error;
    unsigned char *data;
    unsigned int width, height;
    error = lodepng_decode32_file(&data, &width, &height, file_name);
    if (error) {
        fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
    }
    flip_image_vertical(data, width, height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    free(data);
}

void load_png_texture_from_buffer(const char *in, int size) {
    unsigned int error;
    unsigned char *data;
    unsigned int width, height;
    error = lodepng_decode32(&data, &width, &height, (unsigned char *)in, size);
    if (error) {
        fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
    }
    flip_image_vertical(data, width, height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, data);
    free(data);
}

int file_exist(const char *filename) {
  struct stat st;
  int result = stat(filename, &st);
  return result == 0;
}
