#ifndef _cube_h_
#define _cube_h_

void make_cube_faces(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n);

void make_cube(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w);

void make_plant(
    float *vertex, float *normal, float *texture,
    float x, float y, float z, float n, int w, float rotation);

void make_player(
    float *vertex, float *normal, float *texture,
    float x, float y, float z, float rx, float ry);

void make_cube_wireframe(
    float *vertex, float x, float y, float z, float n);

#endif
