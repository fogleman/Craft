#include <math.h>
#include "chunk.h"
#include "cube.h"
#include "item.h"
#include "matrix.h"
#include "util.h"

void make_cube_faces(
                     float *data, char ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float n)
{
    static const float positions[6][4][3] = {
        {{-1, -1, -1}, {-1, -1, +1}, {-1, +1, -1}, {-1, +1, +1}},
        {{+1, -1, -1}, {+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, +1, -1}, {-1, +1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, -1, -1}, {-1, -1, +1}, {+1, -1, -1}, {+1, -1, +1}},
        {{-1, -1, -1}, {-1, +1, -1}, {+1, -1, -1}, {+1, +1, -1}},
        {{-1, -1, +1}, {-1, +1, +1}, {+1, -1, +1}, {+1, +1, +1}}
    };
    static const float normals[6][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, +1, 0},
        {0, -1, 0},
        {0, 0, -1},
        {0, 0, +1}
    };
    static const float uvs[6][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[6][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    static const float flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };
    float *d = data;
    float s = 0.0625;
    float a = 0 + 1 / 2048.0;
    float b = s - 1 / 2048.0;
    int faces[6] = {left, right, top, bottom, front, back};
    int tiles[6] = {wleft, wright, wtop, wbottom, wfront, wback};
    for (int i = 0; i < 6; i++) {
        if (faces[i] == 0) {
            continue;
        }
        float du = (tiles[i] % 16) * s;
        float dv = (tiles[i] / 16) * s;
        int flip = ao[i][0] + ao[i][3] > ao[i][1] + ao[i][2];
        for (int v = 0; v < 6; v++) {
            int j = flip ? flipped[i][v] : indices[i][v];
            *(d++) = n * positions[i][j][0];
            *(d++) = n * positions[i][j][1];
            *(d++) = n * positions[i][j][2];
            *(d++) = normals[i][0];
            *(d++) = normals[i][1];
            *(d++) = normals[i][2];
            *(d++) = du + (uvs[i][j][0] ? b : a);
            *(d++) = dv + (uvs[i][j][1] ? b : a);
            *(d++) = (float)ao[i][j] * 0.03125f;
            *(d++) = 0.0f;
        }
    }
}

void make_rotated_cube(float *data, char ao[6][4],
                       int left, int right, int top, int bottom, int front, int back,
                       float x, float y, float z, float n, float rx, float ry, float rz,
                       int w, const int blocks[256][6]) {
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
        n);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    /* Create rotation transformation */
    mat_rotate(mb, 0, 1, 0, ry);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, 1, 0, 0, rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, 0, 0, 1, rz);
    mat_multiply(ma, mb, ma);
    /* Apply to normals */
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 3, 10);
    /* Create translation transformation */
    mat_translate(mb, x, y, z);
    /* Merge with rotation transformation */
    mat_multiply(ma, mb, ma);
    /* Apply to vertices */
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 0, 10);
}

void make_cube(
               float *data, char ao[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w, const int blocks[256][6])
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
        n);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 0, 10);
}

#define OFF_NORMAL 0
#define OFF_VERTEX 3
#define OFF_X 7
#define OFF_Y 12
#define OFF_Z 17
#define OFF_AO 22
#define OFF_DAMAGE_U 27
#define OFF_DAMAGE_V 31

#define OFF_DU 0
#define OFF_DV 5

void make_cube2(GLuint *data, char ao[6][4],
                int left, int right, int top, int bottom, int front, int back,
                int x, int y, int z, const BlockData block, int damage, const int blocks[256][6]) {
    /*
     * For each corner of the cube, which vertex should be used (see vertex shader)
     */
    static const int corners[6][4] = {
        {0, 1, 2, 5},
        {3, 6, 4, 7},
        {2, 5, 4, 7},
        {0, 1, 3, 6},
        {0, 2, 3, 4},
        {1, 5, 6, 7}};

    /*
     * Texture coordinate map for each vertices in each direction and rotation.
     * When the direction is changed or a rotation occurs (or both), the direction
     * and rotation in which the texture is drawn needs to be changed.
     * This map contains this information for each direction, rotation and vertex.
     */
    static const int uvs[6][4][6][4][2] = {
        { // Direction UP
            { // Rotation IDENTITY (none)
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
            },
            { // Rotation LEFT
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}}
            },
            { // Rotation RIGHT
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}}
            },
            { // Rotation HALF (180 degree)
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
            }
        },
        { // Direction DOWN
            { // Rotation IDENTITY (none)
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}}
            },
            { // Rotation LEFT
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}}
            },
            { // Rotation RIGHT
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}}
            },
            { // Rotation HALF (180 degree)
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}}
            }
        },
        { // Direction RIGHT
            { // Rotation IDENTITY (none)
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}}
            },
            { // Rotation LEFT
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}}
            },
            { // Rotation RIGHT
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}}
            },
            { // Rotation HALF (180 degree)
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}}
            }
        },
        { // Direction LEFT
            { // Rotation IDENTITY (none)
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}}
            },
            { // Rotation LEFT
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}}
            },
            { // Rotation RIGHT
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}}
            },
            { // Rotation HALF (180 degree)
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}}
            }
        },
        { // Direction FORWARD
            { // Rotation IDENTITY (none)
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}}
            },
            { // Rotation LEFT
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}}
            },
            { // Rotation RIGHT
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}}
            },
            { // Rotation HALF (180 degree)
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}}
            }
        },
        { // Direction BACKWARD
            { // Rotation IDENTITY (none)
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}}
            },
            { // Rotation LEFT
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
                {{0, 1}, {1, 1}, {0, 0}, {1, 0}}
            },
            { // Rotation RIGHT
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 1}, {0, 1}, {1, 0}, {0, 0}},
                {{1, 0}, {0, 0}, {1, 1}, {0, 1}}
            },
            { // Rotation HALF (180 degree)
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 0}, {1, 1}, {0, 0}, {0, 1}},
                {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
                {{1, 1}, {1, 0}, {0, 1}, {0, 0}}
            }
        }
    };


    /*
     * Texture map for different directions and rotations.
     * For each direction depending on the rotation, different textures should
     * be used for different faces. This map contain this information for all
     * directions and rotations.
     */
    static const int tex[6][4][6] = {
        { // Direction UP
            {0, 1, 2, 3, 4, 5}, // Rotation IDENTITY (none)
            {5, 4, 2, 3, 0, 1}, // Rotation LEFT
            {4, 5, 2, 3, 1, 0}, // Rotation RIGHT
            {1, 0, 2, 3, 5, 4}  // Rotation HALF (180 degree)
        },
        {  // Direction DOWN
            {1, 0, 3, 2, 4, 5}, // Rotation IDENTITY (none)
            {5, 4, 3, 2, 1, 0}, // Rotation LEFT
            {4, 5, 3, 2, 0, 1}, // Rotation RIGHT
            {0, 1, 3, 2, 5, 4}  // Rotation HALF (180 degree)
        },
        { // Direction RIGHT
            {3, 2, 0, 1, 4, 5}, // Rotation IDENTITY (none)
            {3, 2, 4, 5, 1, 0}, // Rotation LEFT
            {3, 2, 5, 4, 0, 1}, // Rotation RIGHT
            {3, 2, 1, 0, 5, 4}  // Rotation HALF (180 degree)
        },
        {  // Direction LEFT
            {2, 3, 1, 0, 4, 5}, // Rotation IDENTITY (none)
            {2, 3, 5, 4, 1, 0}, // Rotation LEFT
            {2, 3, 4, 5, 0, 1}, // Rotation RIGHT
            {2, 3, 0, 1, 5, 4}  // Rotation HALF (180 degree)
        },
        { // Direction FORWARD
            {0, 1, 5, 4, 2, 3}, // Rotation IDENTITY (none)
            {5, 4, 1, 0, 2, 3}, // Rotation LEFT
            {4, 5, 0, 1, 2, 3}, // Rotation RIGHT
            {1, 0, 4, 5, 2, 3}  // Rotation HALF (180 degree)
        },
        { // Direction BACKWARD
            {0, 1, 4, 5, 3, 2}, // Rotation IDENTITY (none)
            {4, 5, 1, 0, 3, 2}, // Rotation LEFT
            {5, 4, 0, 1, 3, 2}, // Rotation RIGHT
            {1, 0, 5, 4, 3, 2}  // Rotation HALF (180 degree)
        }
     };

    static const int indices[6][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    static const int flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };
    GLuint *d = data;
    int faces[6] = {left, right, top, bottom, front, back};
    int dir = block.direction;
    int rot = block.rotation;
    for (int i = 0; i < 6; i++) {
        if (faces[i] == 0) {
            continue;
        }
        int flip = ao[i][0] + ao[i][3] > ao[i][1] + ao[i][2];
        for (int v = 0; v < 6; v++) {
            int j = flip ? flipped[i][v] : indices[i][v];
            int damage_u = damage + (uvs[dir][rot][i][j][0] ? 1 : 0);
            int damage_v = uvs[dir][rot][i][j][1] ? 1 : 0;
            GLuint d1 = (i << OFF_NORMAL) + (corners[i][j] << OFF_VERTEX) +
                (x << OFF_X) + (y << OFF_Y) + (z << OFF_Z) +
                (ao[i][j] << OFF_AO) + (damage_u << OFF_DAMAGE_U) +
                (damage_v << OFF_DAMAGE_V);
            *(d++) = d1;
            int du = (blocks[block.type][tex[dir][rot][i]] % 16) + (uvs[dir][rot][i][j][0] ? 1 : 0);
            int dv = (blocks[block.type][tex[dir][rot][i]] / 16) + (uvs[dir][rot][i][j][1] ? 1 : 0);
            GLuint d2 = (du << OFF_DU) + (dv << OFF_DV);
            *(d++) = d2;
        }
    }
}

void make_plant(
    GLuint *data, char ao,
    int x, int y, int z, const BlockData block, const int blocks[256][6])
{
    static const int position_index[4][4] = {
        {8, 9, 10, 11},
        {8, 9, 10, 11},
        {12, 13, 14, 15},
        {12, 13, 14, 15}
    };
    static const int normal_index[4] = {0, 1, 4, 5};
    static const int uvs[4][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const int indices[4][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    GLuint *d = data;
    for (int i = 0; i < 4; i++) {
        for (int v = 0; v < 6; v++) {
            int j = indices[i][v];
            GLuint d1 = (normal_index[i] << OFF_NORMAL) + (position_index[i][j] << OFF_VERTEX) +
                (x << OFF_X) + (y << OFF_Y) + (z << OFF_Z) +
                (ao << OFF_AO) + (0 << OFF_DAMAGE_U) +
                (0 << OFF_DAMAGE_V);
            *(d++) = d1;
            int du = (blocks[block.type][i] % 16) + (uvs[i][j][0] ? 1 : 0);
            int dv = (blocks[block.type][i] / 16) + (uvs[i][j][1] ? 1 : 0);
            GLuint d2 = (du << OFF_DU) + (dv << OFF_DV);
            *(d++) = d2;
        }
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

void make_character(
    float *data,
    float x, float y, float n, float m, char c, float z)
{
    float *d = data;
    float s = 0.0625;
    float a = s;
    float b = s * 2;
    int w = c - 32;
    float du = (w % 16) * a;
    float dv = 1 - (w / 16) * b - b;
    *(d++) = x - n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x - n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x - n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;
}
