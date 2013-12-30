#ifndef _clouds_h_
#define _clouds_h_

#include "config.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


typedef struct {
    float x,y,z;        //cloud centre position
    float dx,dy,dz;     //cloud movement
    float sx, sy, sz;   //cloud scale
    float r,g,b;        //cloud colour
    
    int *heightmap;    //height map for boxes
    int hmWidth,hmDepth;
} Cloud;

typedef struct {
    float x_prevailing_winds;
    float z_prevailing_winds;
    int cloud_count;
    Cloud **clouds;
   
    int cloud_vertex_buffer;
    int initial_generation;
} Weather;

typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint colour;
    GLuint matrix;
    GLuint sampler;
    GLuint camera;
    GLuint timer;
    GLuint model;
    GLuint cloudColour;
    GLuint skysampler;
    float time;
} CloudAttrib;


Weather *weather;

void create_clouds();
void update_clouds(float x, float z, float rx, float rz);
void render_clouds(CloudAttrib *attrib,int width, int height, float x, float y, float z, float rx, float ry, float fov, int ortho);
void cleanup_clouds();

void remove_cloud(Cloud *c);
void add_cloud(float player_x, float player_z, float rx, float rz);

#endif
