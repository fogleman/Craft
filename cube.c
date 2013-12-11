#include <math.h>
#include "common.h"
#include "cube.h"
#include "matrix.h"

void make_cube_faces(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float x, float y, float z, float n)
{
    float *v = vertex;
    float *d = normal;
    float *t = texture;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du, dv;
    int w;
    if (left) {
        w = wleft;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(d++) = -1; *(d++) = 0; *(d++) = 0;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (right) {
        w = wright;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(d++) = 1; *(d++) = 0; *(d++) = 0;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
    }
    if (top) {
        w = wtop;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(d++) = 0; *(d++) = 1; *(d++) = 0;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (bottom) {
        w = wbottom;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(d++) = 0; *(d++) = -1; *(d++) = 0;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = a + du; *(t++) = b + dv;
    }
    if (front) {
        w = wfront;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(d++) = 0; *(d++) = 0; *(d++) = -1;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
    if (back) {
        w = wback;
        du = (w % 16) * s; dv = (w / 16) * s;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
        *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
        *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(d++) = 0; *(d++) = 0; *(d++) = 1;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = b + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = a + dv;
        *(t++) = a + du; *(t++) = b + dv;
        *(t++) = b + du; *(t++) = b + dv;
    }
}

void make_cube(
    float *vertex, float *normal, float *texture,
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w)
{
    int wleft, wright, wtop, wbottom, wfront, wback;
    w--;
    wbottom = w;
    wleft = wright = wfront = wback = w + 16;
    wtop = w + 32;
    make_cube_faces(
        vertex, normal, texture,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        x, y, z, n);
}

void make_plant(
    float *vertex, float *normal, float *texture,
    float px, float py, float pz, float n, int w, float rotation)
{
    float *v = vertex;
    float *d = normal;
    float *t = texture;
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
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(d++) = -1; *(d++) = 0; *(d++) = 0;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = b + dv;
    *(t++) = a + du; *(t++) = b + dv;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = b + dv;
    // right
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x; *(v++) = y + n; *(v++) = z + n;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(d++) = 1; *(d++) = 0; *(d++) = 0;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = b + dv;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = b + dv;
    *(t++) = a + du; *(t++) = b + dv;
    // front
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(d++) = 0; *(d++) = 0; *(d++) = -1;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = b + dv;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = b + dv;
    *(t++) = b + du; *(t++) = b + dv;
    // back
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(d++) = 0; *(d++) = 0; *(d++) = 1;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = b + du; *(t++) = b + dv;
    *(t++) = b + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = a + dv;
    *(t++) = a + du; *(t++) = b + dv;
    *(t++) = b + du; *(t++) = b + dv;
    float ma[16];
    float mb[16];
    mat_rotate(mb, 0, 1, 0, RADIANS(-rotation));
    mat_apply(normal, mb, 24);
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, RADIANS(rotation));
    mat_multiply(ma, mb, ma);
    mat_translate(mb, px, py, pz);
    mat_multiply(ma, mb, ma);
    mat_apply(vertex, ma, 24);
}

void make_player(
    float *vertex, float *normal, float *texture,
    float x, float y, float z, float rx, float ry)
{
    make_cube_faces(
        vertex, normal, texture,
        1, 1, 1, 1, 1, 1,
        13, 13, 13 + 32, 13, 13, 13 + 16,
        0, 0, 0, 0.4);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, -rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, cosf(rx), 0, sinf(-rx), ry);
    mat_multiply(ma, mb, ma);
    mat_apply(normal, ma, 36);
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, cosf(rx), 0, sinf(rx), -ry);
    mat_multiply(ma, mb, ma);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(vertex, ma, 36);
}

void make_cube_wireframe(float *vertex, float x, float y, float z, float n) {
    float *v = vertex;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;

    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;

    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z - n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y + n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x + n; *(v++) = y - n; *(v++) = z + n;
    *(v++) = x - n; *(v++) = y - n; *(v++) = z + n;
}
