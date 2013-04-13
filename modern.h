#include <GL/glfw.h>

#define PI 3.14159265359
#define DEGREES(radians) ((radians) * 180 / PI)
#define RADIANS(degrees) ((degrees) * PI / 180)
#define ABS(x) ((x) < 0 ? (-x) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

GLuint make_buffer(GLenum target, GLsizei size, const void *data);
GLuint make_shader(GLenum type, const char *source);
GLuint load_shader(GLenum type, const char *path);
GLuint make_program(GLuint shader1, GLuint shader2);
GLuint load_program(const char *path1, const char *path2);
void identity_matrix(float *matrix);
void frustum_matrix(float *matrix, float left, float right, float bottom,
    float top, float znear, float zfar);
void perspective_matrix(float *matrix, float fov, float aspect,
    float near, float far);
void make_cube(float *vertex, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n);
