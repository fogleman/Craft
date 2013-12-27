#ifndef _clouds_h_
#define _clouds_h_

#include "config.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


enum SEASON {
    SEASON_SUMMER,
    SEASON_AUTUMN,
    SEASON_WINTER,
    SEASON_SPRING
};

enum SEASON_MODIFIER {
    SEASON_M_MILD,
    SEASON_M_NORMAL,
    SEASON_M_SEVERE
};

enum CLOUD_MODE {
    CLOUD_MODE_STABLE,
    CLOUD_MODE_BUILDING,
    CLOUD_MODE_BUILDING_FAST,
    CLOUD_MODE_DECAYING,
    CLOUD_MODE_DECAYING_FAST,
    CLOUD_MODE_DECLINING
};

typedef struct {
    //transform for layer
    float matrix[16];
    
} CloudLayer;


typedef struct {
    float x,y,z;        //cloud centre position
    float dx,dy,dz;     //cloud movement
    
    float r,g,b;        //cloud colour
    
    int *heightmap;    //height map for boxes
    int hmWidth,hmDepth;
    
    int layers_count;
    int cloud_mode;
    int cloud_life;
    int cloud_ticks;
    int *lifetime;
    int lifetimelength;
    int lifetimelength_subcounter;
    CloudLayer **layers;
} Cloud;

typedef struct {
    float x_prevailing_winds;
    float z_prevailing_winds;
    enum SEASON season;
    enum SEASON_MODIFIER season_modifier;
    int season_lifecycle_current;
    int season_lifecycle_max;
    int cloud_count;
    Cloud **clouds;
    
    int cloud_vertex_buffer;
} Weather;

typedef struct {
    GLuint program;
    GLuint position;
    GLuint normal;
    GLuint uv;
    GLuint matrix;
    GLuint sampler;
    GLuint camera;
    GLuint timer;
    GLuint model;
    GLuint cloudColour;
} CloudAttrib;


Weather *weather;

void create_clouds();
void update_clouds(float x, float z);
void render_clouds(CloudAttrib *attrib,int width, int height, float x, float y, float z, float rx, float ry, float fov, int ortho);
void cleanup_clouds();
void remove_cloud(Cloud *c);
void add_cloud(float player_x, float player_z);

#endif
