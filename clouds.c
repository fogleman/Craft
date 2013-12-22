
#include "clouds.h"
#include "noise.h"
#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

void create_clouds() {
    printf("[CLOUDS] Create clouds called\n");
    weather = (Weather*)malloc(sizeof(Weather));
    
    //choose a starting season.
    weather->season = SEASON_SUMMER; //default to summer for now
    weather->season_modifier = SEASON_M_NORMAL; //Normal season
    
    //set a prevailaing wind direction.
    weather->x_prevailing_winds = 1.0f;
    weather->z_prevailing_winds = 0.0f;
    
    //set a lifecycle current and max
    weather->season_lifecycle_current = 0;
    weather->season_lifecycle_max = 10000;
    
    weather->cloud_count = 0;
    weather->clouds = (Cloud**)malloc(MAXIMUM_CLOUDS * sizeof(Cloud*));
    
    float data[144];
    weather->cloud_vertex_buffer = gen_buffer(sizeof(data), data);
    
}




void update_clouds() {
    weather->season_lifecycle_current++;
    
    if(weather->season_lifecycle_current > weather->season_lifecycle_max){
        //season change
        weather->season_lifecycle_current = 0;
        weather->season++;
        printf("[CLOUDS] Season changed");
        
        weather->season = weather->season > SEASON_SPRING ? SEASON_SUMMER : weather->season;
    }
    
    //update prevailing winds
    
    //move clouds with prevailing winds.
    
    //delete clouds that have strayed too far from the player, or have degenerated.
    
    //add new cloud if required.
    add_cloud();
    
}


void add_cloud(){
    //certain types of weather will force less clouds to be allowed.
    int weather_cloud_max_modifier = 0;
    if(weather->cloud_count < MAXIMUM_CLOUDS - weather_cloud_max_modifier){
        weather->clouds[weather->cloud_count] = (Cloud*)malloc(sizeof(Cloud));
        weather->cloud_count++;
        
        
        
        printf("[Cloud] Added new cloud\n");
    }
}


void render_clouds(Attrib *attrib, void (*draw_triangles_3d)(Attrib *attrib, GLuint buffer, int count)) {
    for(i=0; i<weather->cloud_count; i++){
        render_cloud((weather->clouds)[i]);
    }
}

void cleanup_clouds() {
    printf("[CLOUDS] Cleanup clouds called\n");
    printf("[CLOUDS] Removing %d clouds.\n",weather->cloud_count);
    int i;
    for(i=0; i<weather->cloud_count; i++){
        free((weather->clouds)[i]);
    }
    free(weather->clouds);
    
    free(weather);
}