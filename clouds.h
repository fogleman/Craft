#ifndef _clouds_h_
#define _clouds_h_

#include "craftcommonstructs.h"
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
    float dx,dy,dz;
    float sx,sy,sz;
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


Weather *weather;

void create_clouds();
void update_clouds(Player *player);
void render_clouds(Attrib *attrib,int width, int height, Player *player, float fov, int ortho);
void cleanup_clouds();
void remove_cloud(Cloud *c);
void add_cloud(Player *player);

#endif
