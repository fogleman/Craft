#ifndef _util_h_
#define _util_h_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "config.h"

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
    unsigned int fps;
    unsigned int frames;
    double since;
} FPS;

int rand_int(int n);
double rand_double();
void update_fps(FPS *fps);
char *load_file(const char *path);

void load_png_texture(const char *file_name);
char *tokenize(char *str, const char *delim, char **key);
int char_width(char input);
int string_width(const char *input);
int wrap(const char *input, int max_width, char *output, int max_length);

#endif
