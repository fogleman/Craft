#ifndef _cube_h_
#define _cube_h_

#include<gl_includes.h>

void make_cube_faces(
    float *data, char ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n);

void make_cube(
    float *data, char ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w, const int blocks[256][6]);

void make_cube2(
    GLuint *data, char ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int x, int y, int z, int w, int damage, const int blocks[256][6]);

void make_rotated_cube(float *data, char ao[6][4],
                       int left, int right, int top, int bottom, int front, int back,
                       float x, float y, float z, float n, float rx, float ry, float rz,
                       int w, const int blocks[256][6]);

void make_plant(
    GLuint *data, char ao,
    int x, int y, int z, int w, const int blocks[256][6]);

void make_sphere(float *data, float r, int detail);

void make_character(float *data, float x, float y, float n, float m, char c, float z);
#endif
