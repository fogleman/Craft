void make_inventory(float *data, float x, float y, float n, float m, int s) {
    float *d = data;
    float z = 0.5;
    float a = z;
    float b = z * 2;
    int w = s;
    float du = w * a;
    float p = 0;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = p;
    *(d++) = x + n; *(d++) = y - m;
    *(d++) = du + a; *(d++) = p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = b - p;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = b - p;
    *(d++) = x - n; *(d++) = y + m;
    *(d++) = du + 0; *(d++) = b - p;
}
