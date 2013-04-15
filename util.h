#ifdef __MINGW32__
#include <GL/glew.h>
#endif

#include <GL/glfw.h>

#define PI 3.14159265359
#define DEGREES(radians) ((radians) * 180 / PI)
#define RADIANS(degrees) ((degrees) * PI / 180)
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

GLuint make_buffer(GLenum target, GLsizei size, const void *data);
GLuint make_shader(GLenum type, const char *source);
GLuint load_shader(GLenum type, const char *path);
GLuint make_program(GLuint shader1, GLuint shader2);
GLuint load_program(const char *path1, const char *path2);

void normalize(float *x, float *y, float *z);
void mat_identity(float *matrix);
void mat_translate(float *matrix, float dx, float dy, float dz);
void mat_rotate(float *matrix, float x, float y, float z, float angle);
void mat_vec_multiply(float *vector, float *a, float *b);
void mat_multiply(float *matrix, float *a, float *b);
void mat_frustum(float *matrix, float left, float right, float bottom,
    float top, float znear, float zfar);
void mat_perspective(float *matrix, float fov, float aspect,
    float near, float far);

void make_cube(float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w);
