#ifndef _util_h_
#define _util_h_
#include<nanogui/opengl.h>
#include<nanogui/nanogui.h>

#define PI 3.14159265359
#define DEGREES(radians) ((radians) * 180 / PI)
#define RADIANS(degrees) ((degrees) * PI / 180)
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SIGN(x) (((x) > 0) - ((x) < 0))

#if DEBUG
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

typedef struct {
    unsigned int old_fps;
    unsigned int fps;
    unsigned int frames;
    double since;
} FPS;

void update_fps(FPS *fps);

GLuint gen_buffer(GLsizei size, GLfloat *data);
GLuint gen_faces(int components, int faces, GLfloat *data);
void load_png_texture(const char *file_name);
void load_png_texture_from_buffer(const char *in, int size);
int file_exist(const char *filename);

#endif
