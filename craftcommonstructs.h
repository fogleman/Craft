#ifndef _craft_structs_h_
#define _craft_structs_h_
#ifndef __APPLE_CC__
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include "config.h"
#include "map.h"

typedef struct {
    Map map;
    int p;
    int q;
    int faces;
    int dirty;
    GLuint buffer;
} Chunk;

typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint uv;
    GLuint matrix;
    GLuint model;
    GLuint sampler;
    GLuint camera;
    GLuint timer;
    GLuint cloudColour;
} Attrib;

typedef struct {
    float x;
    float y;
    float z;
    float rx;
    float ry;
    float t;
} State;

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    State state;
    State state1;
    State state2;
    GLuint buffer;
} Player;

#endif