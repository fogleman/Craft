#ifndef _clouds_h_
#define _clouds_h_

<<<<<<< HEAD

=======
#include "craftcommonstructs.h"
>>>>>>> committing changes to files. Moved cloud update method in main to use player struct.
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
    float dx,dy,dz;
    float sx,sy,sz;
    int layers_count;
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
    
<<<<<<< HEAD

    GLUint cloud_vertex_buffer;

=======
    int cloud_vertex_buffer;
>>>>>>> committing changes to files. Moved cloud update method in main to use player struct.
} Weather;


Weather *weather;

void create_clouds();
void update_clouds(Player *player);
void render_clouds(Attrib *attrib,int width, int height, Player *player, float fov, int ortho);
void cleanup_clouds();
<<<<<<< HEAD
void remove_cloud(Cloud *c);
=======
>>>>>>> committing changes to files. Moved cloud update method in main to use player struct.
void add_cloud(Player *player);

#endif
