#include <math.h>
#include "cube.h"
#include "item.h"
#include "matrix.h"
#include "util.h"

void make_cube_faces(
    float *data, float ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n)
{
    float *d = data;
    float s = 0.0625;
    float a = 0 + 1 / 2048.0;
    float b = s - 1 / 2048.0;
    float du, dv;
    int w;
    if (left) {
        w = wleft;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[0][0];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[0][3];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[0][2];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[0][0];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[0][1];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[0][3];
    }
    if (right) {
        w = wright;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[1][0];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[1][3];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[1][1];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[1][0];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[1][2];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[1][3];
    }
    if (top) {
        w = wtop;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[2][0];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[2][1];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[2][3];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[2][0];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[2][3];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[2][2];
    }
    if (bottom) {
        w = wbottom;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[3][0];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[3][2];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[3][3];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[3][0];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[3][3];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[3][1];
    }
    if (front) {
        w = wfront;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[4][0];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[4][3];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[4][2];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[4][0];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[4][1];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[4][3];
    }
    if (back) {
        w = wback;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[5][0];
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = ao[5][2];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[5][3];
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = ao[5][0];
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = ao[5][3];
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = ao[5][1];
    }
}

void make_cube(
    float *data, float ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    int wleft = blocks[w][0];
    int wright = blocks[w][1];
    int wtop = blocks[w][2];
    int wbottom = blocks[w][3];
    int wfront = blocks[w][4];
    int wback = blocks[w][5];
    make_cube_faces(
        data, ao,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        x, y, z, n);
}

void make_plant(
    float *data,
    float px, float py, float pz, float n, int w, float rotation)
{
    float *d = data;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du, dv;
    w = plants[w];
    du = (w % 16) * s;
    dv = (w / 16) * s;
    float x, y, z;
    x = y = z = 0;
    // left
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y - n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    // right
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y - n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    // front
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    // back
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = 0;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = 0;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = 0;
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, RADIANS(rotation));
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, 3, 9);
    mat_translate(mb, px, py, pz);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, 0, 9);
}

void make_player(
    float *data,
    float x, float y, float z, float rx, float ry)
{
    float ao[6][4] = {0};
    make_cube_faces(
        data, ao,
        1, 1, 1, 1, 1, 1,
        226, 224, 241, 209, 225, 227,
        0, 0, 0, 0.4);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, cosf(rx), 0, sinf(rx), -ry);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 36, 3, 9);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 36, 0, 9);
}

void make_cube_wireframe(float *data, float x, float y, float z, float n) {
    float *d = data;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
}

void make_character(
    float *data,
    float x, float y, float n, float m, char c)
{
    float *d = data;
    float s = 0.0625;
    float a = s;
    float b = s * 2;
    int w = c - 32;
    float du = (w % 16) * a;
    float dv = 1 - (w / 16) * b - b;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = x + n; *(d++) = y - m;
    *(d++) = du + a; *(d++) = dv;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = x - n; *(d++) = y + m;
    *(d++) = du + 0; *(d++) = dv + b;
}

void make_character_3d(
    float *data,
    float x, float y, float z, float n, float m, int face, char c)
{
    float *d = data;
    float s = 0.0625;
    float pu = s / 5;
    float pv = s / 2.5;
    float u1 = pu;
    float v1 = pv;
    float u2 = s - pu;
    float v2 = s * 2 - pv;
    float p = 0.5;
    int w = c - 32;
    float du = (w % 16) * s;
    float dv = 1 - (w / 16 + 1) * s * 2;
    if (face == 0) { // left
        x -= p;
        *(d++) = x; *(d++) = y - m; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y + m; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x; *(d++) = y + m; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x; *(d++) = y - m; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y - m; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y + m; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v2;
    }
    if (face == 1) { // right
        x += p;
        *(d++) = x; *(d++) = y - m; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y + m; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x; *(d++) = y - m; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y - m; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x; *(d++) = y + m; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x; *(d++) = y + m; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v2;
    }
    if (face == 2) { // front
        z -= p;
        *(d++) = x - n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x + n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x - n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x - n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x + n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v2;
    }
    if (face == 3) { // back
        z += p;
        *(d++) = x - n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x - n; *(d++) = y - m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x - n; *(d++) = y + m; *(d++) = z;
        *(d++) = du + u1; *(d++) = dv + v2;
    }
    if (face == 4) { // top 1
        y += p;
        *(d++) = x - n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x - n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x + n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x - n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u1; *(d++) = dv + v2;
    }
    if (face == 5) { // top 2
        y += p;
        *(d++) = x - m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x + m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x - m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x - m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x + m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x + m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u2; *(d++) = dv + v1;
    }
    if (face == 6) { // top 3
        y += p;
        *(d++) = x + n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x - n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x - n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x + n; *(d++) = y; *(d++) = z + m;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x + n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x - n; *(d++) = y; *(d++) = z - m;
        *(d++) = du + u2; *(d++) = dv + v1;
    }
    if (face == 7) { // top 4
        y += p;
        *(d++) = x + m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x - m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v1;
        *(d++) = x + m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v2;
        *(d++) = x + m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v2;
        *(d++) = x - m; *(d++) = y; *(d++) = z - n;
        *(d++) = du + u1; *(d++) = dv + v1;
        *(d++) = x - m; *(d++) = y; *(d++) = z + n;
        *(d++) = du + u2; *(d++) = dv + v1;
    }
}

int _make_sphere(
    float *data, float r, int detail,
    float *a, float *b, float *c,
    float *ta, float *tb, float *tc)
{
    if (detail == 0) {
        float *d = data;
        *(d++) = a[0] * r; *(d++) = a[1] * r; *(d++) = a[2] * r;
        *(d++) = a[0]; *(d++) = a[1]; *(d++) = a[2];
        *(d++) = ta[0]; *(d++) = ta[1];
        *(d++) = b[0] * r; *(d++) = b[1] * r; *(d++) = b[2] * r;
        *(d++) = b[0]; *(d++) = b[1]; *(d++) = b[2];
        *(d++) = tb[0]; *(d++) = tb[1];
        *(d++) = c[0] * r; *(d++) = c[1] * r; *(d++) = c[2] * r;
        *(d++) = c[0]; *(d++) = c[1]; *(d++) = c[2];
        *(d++) = tc[0]; *(d++) = tc[1];
        return 1;
    }
    else {
        float ab[3], ac[3], bc[3];
        for (int i = 0; i < 3; i++) {
            ab[i] = (a[i] + b[i]) / 2;
            ac[i] = (a[i] + c[i]) / 2;
            bc[i] = (b[i] + c[i]) / 2;
        }
        normalize(ab + 0, ab + 1, ab + 2);
        normalize(ac + 0, ac + 1, ac + 2);
        normalize(bc + 0, bc + 1, bc + 2);
        float tab[2], tac[2], tbc[2];
        tab[0] = 0; tab[1] = 1 - acosf(ab[1]) / PI;
        tac[0] = 0; tac[1] = 1 - acosf(ac[1]) / PI;
        tbc[0] = 0; tbc[1] = 1 - acosf(bc[1]) / PI;
        int total = 0;
        int n;
        n = _make_sphere(data, r, detail - 1, a, ab, ac, ta, tab, tac);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, b, bc, ab, tb, tbc, tab);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, c, ac, bc, tc, tac, tbc);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, ab, bc, ac, tab, tbc, tac);
        total += n; data += n * 24;
        return total;
    }
}

void make_sphere(float *data, float r, int detail) {
    // detail, triangles, floats
    // 0, 8, 192
    // 1, 32, 768
    // 2, 128, 3072
    // 3, 512, 12288
    // 4, 2048, 49152
    // 5, 8192, 196608
    // 6, 32768, 786432
    // 7, 131072, 3145728
    static int indices[8][3] = {
        {4, 3, 0}, {1, 4, 0},
        {3, 4, 5}, {4, 1, 5},
        {0, 3, 2}, {0, 2, 1},
        {5, 2, 3}, {5, 1, 2}
    };
    static float positions[6][3] = {
        { 0, 0,-1}, { 1, 0, 0},
        { 0,-1, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0, 0, 1}
    };
    static float uvs[6][3] = {
        {0, 0.5}, {0, 0.5},
        {0, 0}, {0, 0.5},
        {0, 1}, {0, 0.5}
    };
    int total = 0;
    for (int i = 0; i < 8; i++) {
        int n = _make_sphere(
            data, r, detail,
            positions[indices[i][0]],
            positions[indices[i][1]],
            positions[indices[i][2]],
            uvs[indices[i][0]],
            uvs[indices[i][1]],
            uvs[indices[i][2]]);
        total += n; data += n * 24;
    }
}
