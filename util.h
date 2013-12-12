#ifndef _util_h_
#define _util_h_

#ifndef __APPLE_CC__
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

typedef struct {
    unsigned int frames;
    double since;
} FPS;

int rand_int(int n);
double rand_double();
void update_fps(FPS *fps, int show);

void malloc_buffers(
    int faces,
    GLfloat **position_data, GLfloat **normal_data, GLfloat **uv_data);
GLuint gen_buffer(GLenum target, GLsizei size, const void *data);
void gen_buffers(
    int faces,
    GLfloat *position_data, GLfloat *normal_data, GLfloat *uv_data,
    GLuint *position_buffer, GLuint *normal_buffer, GLuint *uv_buffer);
GLuint make_shader(GLenum type, const char *source);
GLuint load_shader(GLenum type, const char *path);
GLuint make_program(GLuint shader1, GLuint shader2);
GLuint load_program(const char *path1, const char *path2);
void load_png_texture(const char *file_name);

#endif
