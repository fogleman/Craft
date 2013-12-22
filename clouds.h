#ifndef _clouds_h_
#define _clouds_h_

#include "config.h"

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

typedef struct {
    //transform for layer
    float matrix[16];
    
} CloudLayer;


typedef struct {
    float x,y,z;        //cloud centre position
    int layers;
    GLUint buffer;
    GLUint faces;
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
    
    GLUint cloud_vertex_buffer;
} Weather;


Weather *weather;

void create_clouds();
void update_clouds();
void render_clouds(Attrib *attrib, void (*draw_triangles_3d)(Attrib *attrib, GLuint buffer, int count));
void cleanup_clouds();
void add_cloud();

#endif