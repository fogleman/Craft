#ifndef _util_h_
#define _util_h_

#ifndef __APPLE_CC__
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#define PI 3.14159265359
#define DEGREES(radians) ((radians) * 180 / PI)
#define RADIANS(degrees) ((degrees) * PI / 180)
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    unsigned int frames;
    double since;
} FPS;

int rand_int(int n);
double rand_double();
void update_fps(FPS *fps, int show);

/* Case insensitive string n-comparison.
 * Returns 0 if the first n characters of *a and *b are equal.
 *       < 0 if *a[0..n] < *b[0..n]
 *       > 0 if *a[0..n] > *b[0..n]
 * If strlen(*a) < strlen(*b), *a compares less than *b.
 * There is no standard platform agnostic version of this.
 */
int strncicmp(const char *a, const char *b, size_t n);

void malloc_buffers(
    int components, int faces,
    GLfloat **position_data, GLfloat **normal_data, GLfloat **uv_data);
GLuint gen_buffer(GLenum target, GLsizei size, const void *data);
void gen_buffers(
    int components, int faces,
    GLfloat *position_data, GLfloat *normal_data, GLfloat *uv_data,
    GLuint *position_buffer, GLuint *normal_buffer, GLuint *uv_buffer);
GLuint make_shader(GLenum type, const char *source);
GLuint load_shader(GLenum type, const char *path);
GLuint make_program(GLuint shader1, GLuint shader2);
GLuint load_program(const char *path1, const char *path2);
void load_png_texture(const char *file_name);

#endif
