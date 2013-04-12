#include <math.h>
#include <stdlib.h>

#define INDEX(size, x, y) ((y) * (size) + (x))

double noise(double roughness) {
    double result = (double)rand() / RAND_MAX;
    result = result * 2 - 1;
    result = result * roughness;
    return result;
}

void plasma(int size, double roughness, double *data) {
    int n = size - 1;
    int length = size - 1;
    data[INDEX(size, 0, 0)] = 0.5;
    data[INDEX(size, n, 0)] = 0.5;
    data[INDEX(size, 0, n)] = 0.5;
    data[INDEX(size, n, n)] = 0.5;
    while (length > 1) {
        int half = length / 2;
        for (int x = 0; x < n; x += length) {
            for (int y = 0; y < n; y += length) {
                double v1 = data[INDEX(size, x, y)];
                double v2 = data[INDEX(size, x + length, y)];
                double v3 = data[INDEX(size, x, y + length)];
                double v4 = data[INDEX(size, x + length, y + length)];
                double v = (v1 + v2 + v3 + v4) / 4 + noise(roughness);
                data[INDEX(size, x + half, y + half)] = v;
            }
        }
        for (int x = 0; x < n; x += half) {
            int a = (x + half) % length;
            for (int y = a; y < n; y += length) {
                double v1 = data[INDEX(size, (x - half + n) % n, y)];
                double v2 = data[INDEX(size, (x + half) % n, y)];
                double v3 = data[INDEX(size, x, (y - half + n) % n)];
                double v4 = data[INDEX(size, x, (y + half) % n)];
                double v = (v1 + v2 + v3 + v4) / 4 + noise(roughness);
                data[INDEX(size, x, y)] = v;
                if (x == 0) {
                    data[INDEX(size, n, y)] = v;
                }
                if (y == 0) {
                    data[INDEX(size, x, n)] = v;
                }
            }
        }
        roughness /= 2;
        length /= 2;
    }
    int area = size * size;
    double lo = data[0];
    double hi = data[0];
    for (int i = 1; i < area; i++) {
        lo = data[i] < lo ? data[i] : lo;
        hi = data[i] > hi ? data[i] : hi;
    }
    for (int i = 0; i < area; i++) {
        double p = (data[i] - lo) / (hi - lo);
        data[i] = p;
    }
}
