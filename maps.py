'''
Sweeeeeeet
'''

#include <stdlib.h>
#include "map.h"

def hash_int(key):
    key = ~key + (key << 15)
    key = key ^ (key >> 12)
    key = key + (key << 2)
    key = key ^ (key >> 4)
    key = key * 2057
    key = key ^ (key >> 16)
    return key

def hash(x, y, z):
    x = hash_int(x)
    y = hash_int(y)
    z = hash_int(z)
    return x ^ y ^ z

def map_set(map, x, y, z, w):
    index = hash(x, y, z) & map.mask
    entry = map.data[index]
    overwrite = 0
    while (not EMPTY_ENTRY(entry)):
        if (entry.x == x and entry.y == y and entry.z == z):
            overwrite = 1
            break
        index = (index + 1) & map.mask
        entry = map.data[index]
    if (overwrite):
        entry.w = w
    elif (w):
        entry.x = x
        entry.y = y
        entry.z = z
        entry.w = w
        map.size += 1
        if (map.size * 2 > map.mask):
            map_grow(map)


'''
def map_get(map, int x, int y, int z) {
    unsigned int index = hash(x, y, z) & map->mask;
    Entry *entry = map->data + index;
    while (!EMPTY_ENTRY(entry)) {
        if (entry->x == x && entry->y == y && entry->z == z) {
            return entry->w;
        }
        index = (index + 1) & map->mask;
        entry = map->data + index;
    }
    return 0;
}

void map_grow(Map *map) {
    Map new_map;
    new_map.mask = (map->mask << 1) | 1;
    new_map.size = 0;
    new_map.data = (Entry *)calloc(new_map.mask + 1, sizeof(Entry));
    MAP_FOR_EACH(map, entry) {
        map_set(&new_map, entry->x, entry->y, entry->z, entry->w);
    } END_MAP_FOR_EACH;
    free(map->data);
    map->mask = new_map.mask;
    map->size = new_map.size;
    map->data = new_map.data;
}

void create_world(Map *map, int p, int q) {
    int pad = 1;
    for (int dx = -pad; dx < CHUNK_SIZE + pad; dx++) {
        for (int dz = -pad; dz < CHUNK_SIZE + pad; dz++) {
            int x = p * CHUNK_SIZE + dx;
            int z = q * CHUNK_SIZE + dz;
            float f = simplex2(x * 0.01, z * 0.01, 4, 0.5, 2);
            float g = simplex2(-x * 0.01, -z * 0.01, 2, 0.9, 2);
            int mh = g * 32 + 16;
            int h = f * mh;
            int w = 1;
            int t = 12;
            if (h <= t) {
                h = t;
                w = 2;
            }
            if (dx < 0 || dz < 0 || dx >= CHUNK_SIZE || dz >= CHUNK_SIZE) {
                w = -1;
            }
            // sand and grass terrain
            for (int y = 0; y < h; y++) {
                map_set(map, x, y, z, w);
            }
            // TODO: w = -1 if outside of chunk
            if (w == 1) {
                // grass
                if (simplex2(-x * 0.1, z * 0.1, 4, 0.8, 2) > 0.6) {
                    map_set(map, x, h, z, 17);
                }
                // flowers
                if (simplex2(x * 0.05, -z * 0.05, 4, 0.8, 2) > 0.7) {
                    int w = 18 + simplex2(x * 0.1, z * 0.1, 4, 0.8, 2) * 7;
                    map_set(map, x, h, z, w);
                }
                // trees
                int ok = 1;
                if (dx - 4 < 0 || dz - 4 < 0 ||
                    dx + 4 >= CHUNK_SIZE || dz + 4 >= CHUNK_SIZE)
                    {
                        ok = 0;
                    }
                    if (ok && simplex2(x, z, 6, 0.5, 2) > 0.84) {
                        for (int y = h + 3; y < h + 8; y++) {
                            for (int ox = -3; ox <= 3; ox++) {
                                for (int oz = -3; oz <= 3; oz++) {
                                    int d = (ox * ox) + (oz * oz) +
                                        (y - (h + 4)) * (y - (h + 4));
                                    if (d < 11) {
                                        map_set(map, x + ox, y, z + oz, 15);
                            }
                        }
                        }
                        }
                        for (int y = h; y < h + 7; y++) {
                            map_set(map, x, y, z, 5);
                    }
            }
            }
            // clouds
            for (int y = 64; y < 72; y++) {
                if (simplex3(x * 0.01, y * 0.1, z * 0.01, 8, 0.5, 2) > 0.75) {
                    map_set(map, x, y, z, 16);
        }
    }
}
}
}
'''

F2 = 0.3660254037844386
G2 = 0.21132486540518713
F3 = (1.0 / 3.0)
G3 = (1.0 / 6.0)
def ASSIGN(a, v0, v1, v2):
    (a)[0] = v0
    (a)[1] = v1
    (a)[2] = v2

def DOT3(v1, v2):
    return ((v1)[0] * (v2)[0] + (v1)[1] * (v2)[1] + (v1)[2] * (v2)[2])

GRAD3 = [
    [ 1, 1, 0], [-1, 1, 0], [ 1,-1, 0], [-1,-1, 0],
    [ 1, 0, 1], [-1, 0, 1], [ 1, 0,-1], [-1, 0,-1],
    [ 0, 1, 1], [ 0,-1, 1], [ 0, 1,-1], [ 0,-1,-1],
    [ 1, 0,-1], [-1, 0,-1], [ 0,-1, 1], [ 0, 1, 1]
]

PERM = [
    151, 160, 137,  91,  90,  15, 131,  13,
    201,  95,  96,  53, 194, 233,   7, 225,
    140,  36, 103,  30,  69, 142,   8,  99,
    37, 240,  21,  10,  23, 190,   6, 148,
    247, 120, 234,  75,   0,  26, 197,  62,
    94, 252, 219, 203, 117,  35,  11,  32,
    57, 177,  33,  88, 237, 149,  56,  87,
    174,  20, 125, 136, 171, 168,  68, 175,
    74, 165,  71, 134, 139,  48,  27, 166,
    77, 146, 158, 231,  83, 111, 229, 122,
    60, 211, 133, 230, 220, 105,  92,  41,
    55,  46, 245,  40, 244, 102, 143,  54,
    65,  25,  63, 161,   1, 216,  80,  73,
    209,  76, 132, 187, 208,  89,  18, 169,
    200, 196, 135, 130, 116, 188, 159,  86,
    164, 100, 109, 198, 173, 186,   3,  64,
    52, 217, 226, 250, 124, 123,   5, 202,
    38, 147, 118, 126, 255,  82,  85, 212,
    207, 206,  59, 227,  47,  16,  58,  17,
    182, 189,  28,  42, 223, 183, 170, 213,
    119, 248, 152,   2,  44, 154, 163,  70,
    221, 153, 101, 155, 167,  43, 172,   9,
    129,  22,  39, 253,  19,  98, 108, 110,
    79, 113, 224, 232, 178, 185, 112, 104,
    218, 246,  97, 228, 251,  34, 242, 193,
    238, 210, 144,  12, 191, 179, 162, 241,
    81,  51, 145, 235, 249,  14, 239, 107,
    49, 192, 214,  31, 181, 199, 106, 157,
    184,  84, 204, 176, 115, 121,  50,  45,
    127,   4, 150, 254, 138, 236, 205,  93,
    222, 114,  67,  29,  24,  72, 243, 141,
    128, 195,  78,  66, 215,  61, 156, 180,
    151, 160, 137,  91,  90,  15, 131,  13,
    201,  95,  96,  53, 194, 233,   7, 225,
    140,  36, 103,  30,  69, 142,   8,  99,
    37, 240,  21,  10,  23, 190,   6, 148,
    247, 120, 234,  75,   0,  26, 197,  62,
    94, 252, 219, 203, 117,  35,  11,  32,
    57, 177,  33,  88, 237, 149,  56,  87,
    174,  20, 125, 136, 171, 168,  68, 175,
    74, 165,  71, 134, 139,  48,  27, 166,
    77, 146, 158, 231,  83, 111, 229, 122,
    60, 211, 133, 230, 220, 105,  92,  41,
    55,  46, 245,  40, 244, 102, 143,  54,
    65,  25,  63, 161,   1, 216,  80,  73,
    209,  76, 132, 187, 208,  89,  18, 169,
    200, 196, 135, 130, 116, 188, 159,  86,
    164, 100, 109, 198, 173, 186,   3,  64,
    52, 217, 226, 250, 124, 123,   5, 202,
    38, 147, 118, 126, 255,  82,  85, 212,
    207, 206,  59, 227,  47,  16,  58,  17,
    182, 189,  28,  42, 223, 183, 170, 213,
    119, 248, 152,   2,  44, 154, 163,  70,
    221, 153, 101, 155, 167,  43, 172,   9,
    129,  22,  39, 253,  19,  98, 108, 110,
    79, 113, 224, 232, 178, 185, 112, 104,
    218, 246,  97, 228, 251,  34, 242, 193,
    238, 210, 144,  12, 191, 179, 162, 241,
    81,  51, 145, 235, 249,  14, 239, 107,
    49, 192, 214,  31, 181, 199, 106, 157,
    184,  84, 204, 176, 115, 121,  50,  45,
    127,   4, 150, 254, 138, 236, 205,  93,
    222, 114,  67,  29,  24,  72, 243, 141,
    128, 195,  78,  66, 215,  61, 156, 180
]

def noise2(x, y):
    s = (x + y) * F2
    i = math.floor(x + s)
    j = math.floor(y + s)
    t = (i + j) * G2
    
    xx = []
    yy = []
    f = []
    noise = [0.0, 0.0, 0.0]
    g = []
    
    xx[0] = x - (i - t)
    yy[0] = y - (j - t)
    
    i1 = xx[0] > yy[0]
    j1 = xx[0] <= yy[0]
    
    xx[2] = xx[0] + G2 * 2.0 - 1.0
    yy[2] = yy[0] + G2 * 2.0 - 1.0
    xx[1] = xx[0] - i1 + G2
    yy[1] = yy[0] - j1 + G2
    
    I = int(i) & 255
    J = int(j) & 255
    g[0] = PERM[I + PERM[J]] % 12
    g[1] = PERM[I + i1 + PERM[J + j1]] % 12
    g[2] = PERM[I + 1 + PERM[J + 1]] % 12
    
    for c in xrange(3):
        f[c] = 0.5 - xx[c]*xx[c] - yy[c]*yy[c]
    
    for c in xrange(3):
        if (f[c] > 0):
            noise[c] = f[c] * f[c] * f[c] * f[c] * (GRAD3[g[c]][0] * xx[c] + GRAD3[g[c]][1] * yy[c])
    
    return (noise[0] + noise[1] + noise[2]) * 70.0

def noise3(x, y, z):
    o1 = []
    o2 = []
    g = []
    f = []
    noise = [0.0, 0.0, 0.0, 0.0]
    s = (x + y + z) * F3
    i = math.floor(x + s)
    j = math.floor(y + s)
    k = math.floor(z + s)
    t = (i + j + k) * G3
    
    pos = [[]]
    
    pos[0][0] = x - (i - t)
    pos[0][1] = y - (j - t)
    pos[0][2] = z - (k - t)
    
    if (pos[0][0] >= pos[0][1]):
        if (pos[0][1] >= pos[0][2]):
            ASSIGN(o1, 1, 0, 0)
            ASSIGN(o2, 1, 1, 0)
        elif (pos[0][0] >= pos[0][2]):
            ASSIGN(o1, 1, 0, 0)
            ASSIGN(o2, 1, 0, 1)
        else:
            ASSIGN(o1, 0, 0, 1)
            ASSIGN(o2, 1, 0, 1)
    else:
        if (pos[0][1] < pos[0][2]):
            ASSIGN(o1, 0, 0, 1)
            ASSIGN(o2, 0, 1, 1)
        elif (pos[0][0] < pos[0][2]):
            ASSIGN(o1, 0, 1, 0)
            ASSIGN(o2, 0, 1, 1)
        else:
            ASSIGN(o1, 0, 1, 0)
            ASSIGN(o2, 1, 1, 0)
    
    for c in xrange(3):
        pos[3][c] = pos[0][c] - 1.0 + 3.0 * G3
        pos[2][c] = pos[0][c] - o2[c] + 2.0 * G3
        pos[1][c] = pos[0][c] - o1[c] + G3
    
    I = int(i) & 255
    J = int(j) & 255
    K = int(k) & 255
    g[0] = PERM[I + PERM[J + PERM[K]]] % 12
    g[1] = PERM[I + o1[0] + PERM[J + o1[1] + PERM[o1[2] + K]]] % 12
    g[2] = PERM[I + o2[0] + PERM[J + o2[1] + PERM[o2[2] + K]]] % 12
    g[3] = PERM[I + 1 + PERM[J + 1 + PERM[K + 1]]] % 12
    
    for c in xrange(4):
        f[c] = 0.6 - pos[c][0] * pos[c][0] - pos[c][1] * pos[c][1] - pos[c][2] * pos[c][2]
    
    for c in xrange(4):
        if (f[c] > 0):
            noise[c] = f[c] * f[c] * f[c] * f[c] * DOT3(pos[c], GRAD3[g[c]])
    
    return (noise[0] + noise[1] + noise[2] + noise[3]) * 32.0

def simplex2(x, y, octaves, persistence, lacunarity):
    freq = 1.0
    amp = 1.0
    max = 1.0
    total = noise2(x, y)
    for i in xrange(1, octaves):
        freq *= lacunarity
        amp *= persistence
        max += amp
        total += noise2(x * freq, y * freq) * amp
    return (1 + total / max) / 2

def simplex3(x, y, z, octaves, persistence, lacunarity):
    freq = 1.0
    amp = 1.0
    max = 1.0
    total = noise3(x, y, z)
    for i in xrange(1, octaves):
        freq *= lacunarity
        amp *= persistence
        max += amp

    total += noise3(x * freq, y * freq, z * freq) * amp
    return (1 + total / max) / 2