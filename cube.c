#include <math.h>
#include "cube.h"
#include "matrix.h"
#include "util.h"

void make_cube_faces(
    float *data,
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n)
{
    float *d = data;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du, dv;
    int w;
    if (left) {
        w = wleft;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
    }
    if (right) {
        w = wright;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
    }
    if (top) {
        w = wtop;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
    }
    if (bottom) {
        w = wbottom;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = a + du; *(d++) = b + dv;
    }
    if (front) {
        w = wfront;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = b + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = b + du; *(d++) = b + dv;
    }
    if (back) {
        w = wback;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y - n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = a + dv;
        *(d++) = x + n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = a + du; *(d++) = b + dv;
        *(d++) = x - n; *(d++) = y + n; *(d++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = b + du; *(d++) = b + dv;
    }
}

void make_cube(
    float *data,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    int wleft, wright, wtop, wbottom, wfront, wback;
    w--;
    wbottom = w;
    wleft = wright = wfront = wback = w + 16;
    wtop = w + 32;
    make_cube_faces(
        data,
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
    w--;
    du = (w % 16) * s;
    dv = (w / 16 * 3) * s;
    float x, y, z;
    x = y = z = 0;
    // left
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y - n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    // right
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = x; *(d++) = y - n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y - n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z - n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = x; *(d++) = y + n; *(d++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = a + du; *(d++) = b + dv;
    // front
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = b + dv;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = b + du; *(d++) = b + dv;
    // back
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x + n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = a + dv;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = x - n; *(d++) = y - n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = a + dv;
    *(d++) = x + n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = a + du; *(d++) = b + dv;
    *(d++) = x - n; *(d++) = y + n; *(d++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = b + du; *(d++) = b + dv;
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, RADIANS(rotation));
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, sizeof(GLfloat) * 3, sizeof(GLfloat) * 8);
    mat_translate(mb, px, py, pz);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, sizeof(GLfloat) * 0, sizeof(GLfloat) * 8);
}

void make_player(
    float *data,
    float x, float y, float z, float rx, float ry)
{
    make_cube_faces(
        data,
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
    mat_apply(data, ma, 36, sizeof(GLfloat) * 3, sizeof(GLfloat) * 8);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 36, sizeof(GLfloat) * 0, sizeof(GLfloat) * 8);
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
    float p = 0;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv + p;
    *(d++) = x + n; *(d++) = y - m;
    *(d++) = du + a; *(d++) = dv + p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b - p;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = dv + p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = dv + b - p;
    *(d++) = x - n; *(d++) = y + m;
    *(d++) = du + 0; *(d++) = dv + b - p;
}
