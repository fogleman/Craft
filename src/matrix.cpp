#include <math.h>
#include <stdio.h>
#include <nanogui/glutil.h>
#include "matrix.h"
#include "util.h"

#define FAR_PLANE 1000
#define NEAR_PLANE 0.25
#define FOV 60

namespace konstructs {
    namespace matrix {
        using namespace Eigen;
        using namespace nanogui;

        Matrix4f projection(int width, int height) {
            float aspect_ratio = (float)width / (float)height;
            float y_scale = (1.0f / tanf((FOV / 2.0f) * PI / 360.0)) * aspect_ratio;
            float x_scale = y_scale / aspect_ratio;
            float frustrum_length = FAR_PLANE - NEAR_PLANE;

            Matrix4f m;
            m <<
                x_scale, 0.0, 0.0, 0.0,
                0.0, y_scale, 0.0, 0.0,
                0.0, 0.0, -((FAR_PLANE + NEAR_PLANE) / frustrum_length), -((2 * NEAR_PLANE * FAR_PLANE) / frustrum_length),
                0.0, 0.0, -1.0, 0.0;
            return m;
        }

        Matrix4f projection_2d(const int width, const int height) {
            Matrix4f mvp;
            mvp.setIdentity();
            mvp.row(0) *= (float) height / (float) width;
            return mvp;
        }

        Matrix4f projection_frustum(float left, float right, float bottom,
                                    float top, float znear, float zfar) {
            float temp, temp2, temp3, temp4;
            temp = 2.0 * znear;
            temp2 = right - left;
            temp3 = top - bottom;
            temp4 = zfar - znear;
            Matrix4f m;

            m <<
                temp / temp2, 0.0, (right + left) / temp2, 0.0,
                0.0, temp / temp3, (top + bottom) / temp3, 0.0,
                0.0, 0.0, (-zfar - znear) / temp4, (-temp * zfar) / temp4,
                0.0, 0.0, -1.0, 0.0;
            return m;
        }

        Matrix4f projection_perspective(float fov, float aspect,
                                        float znear, float zfar) {
            float ymax, xmax;
            ymax = znear * tanf(fov * PI / 360.0);
            xmax = ymax * aspect;
            return frustum(-xmax, xmax, -ymax, ymax, znear, zfar);
        }

        void ext_frustum_planes(float planes[6][4], int radius, const Matrix4f &m) {
            float znear = 0.125;
            float zfar = radius * 32 + 64;
            planes[0][0] = m(3) + m(0);
            planes[0][1] = m(7) + m(4);
            planes[0][2] = m(11) + m(8);
            planes[0][3] = m(15) + m(12);
            planes[1][0] = m(3) - m(0);
            planes[1][1] = m(7) - m(4);
            planes[1][2] = m(11) - m(8);
            planes[1][3] = m(15) - m(12);
            planes[2][0] = m(3) + m(1);
            planes[2][1] = m(7) + m(5);
            planes[2][2] = m(11) + m(9);
            planes[2][3] = m(15) + m(13);
            planes[3][0] = m(3) - m(1);
            planes[3][1] = m(7) - m(5);
            planes[3][2] = m(11) - m(9);
            planes[3][3] = m(15) - m(13);
            planes[4][0] = znear * m(3) + m(2);
            planes[4][1] = znear * m(7) + m(6);
            planes[4][2] = znear * m(11) + m(10);
            planes[4][3] = znear * m(15) + m(14);
            planes[5][0] = zfar * m(3) - m(2);
            planes[5][1] = zfar * m(7) - m(6);
            planes[5][2] = zfar * m(11) - m(10);
            planes[5][3] = zfar * m(15) - m(14);
        }
    };
};

void normalize(float *x, float *y, float *z) {
    float d = sqrtf((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    *x /= d; *y /= d; *z /= d;
}

void mat_identity(float *matrix) {
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void mat_translate(float *matrix, float dx, float dy, float dz) {
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = dx;
    matrix[13] = dy;
    matrix[14] = dz;
    matrix[15] = 1;
}

void mat_rotate(float *matrix, float x, float y, float z, float angle) {
    normalize(&x, &y, &z);
    float s = sinf(angle);
    float c = cosf(angle);
    float m = 1 - c;
    matrix[0] = m * x * x + c;
    matrix[1] = m * x * y - z * s;
    matrix[2] = m * z * x + y * s;
    matrix[3] = 0;
    matrix[4] = m * x * y + z * s;
    matrix[5] = m * y * y + c;
    matrix[6] = m * y * z - x * s;
    matrix[7] = 0;
    matrix[8] = m * z * x - y * s;
    matrix[9] = m * y * z + x * s;
    matrix[10] = m * z * z + c;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

void mat_vec_multiply(float *vector, float *a, float *b) {
    float result[4];
    for (int i = 0; i < 4; i++) {
        float total = 0;
        for (int j = 0; j < 4; j++) {
            int p = j * 4 + i;
            int q = j;
            total += a[p] * b[q];
        }
        result[i] = total;
    }
    for (int i = 0; i < 4; i++) {
        vector[i] = result[i];
    }
}

void mat_multiply(float *matrix, float *a, float *b) {
    float result[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            int index = c * 4 + r;
            float total = 0;
            for (int i = 0; i < 4; i++) {
                int p = i * 4 + r;
                int q = c * 4 + i;
                total += a[p] * b[q];
            }
            result[index] = total;
        }
    }
    for (int i = 0; i < 16; i++) {
        matrix[i] = result[i];
    }
}

void mat_apply(float *data, float *matrix, int count, int offset, int stride) {
    float vec[4] = {0, 0, 0, 1};
    for (int i = 0; i < count; i++) {
        float *d = data + offset + stride * i;
        vec[0] = *(d++); vec[1] = *(d++); vec[2] = *(d++);
        mat_vec_multiply(vec, matrix, vec);
        d = data + offset + stride * i;
        *(d++) = vec[0]; *(d++) = vec[1]; *(d++) = vec[2];
    }
}
