
#include "clouds.h"
#include "noise.h"
#include "config.h"
#include "util.h"
#include "cube.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include "matrix.h"

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



void update_clouds(float player_x, float player_z) {
    weather->season_lifecycle_current++;
    
    if(weather->season_lifecycle_current > weather->season_lifecycle_max){
        //season change
        weather->season_lifecycle_current = 0;
        weather->season++;
        printf("[CLOUDS] Season changed");
        
        weather->season = weather->season > SEASON_SPRING ? SEASON_SUMMER : weather->season;
    }
    
    //update prevailing winds
    int i;
    for(i=0; i<weather->cloud_count; i++){
        
        Cloud *c = (weather->clouds)[i];
        //delete clouds that have strayed too far from the player, or have degenerated.
        
        if ((pow(player_x - (((weather->clouds)[i])->x),2) + pow(player_z - (((weather->clouds)[i])->z),2)) > pow(500,2)  || c->cloud_life <= 0) {
            
            
            remove_cloud(c);
            weather->clouds[i] = NULL;

            
            int j;
            for (j=i+1; j<weather->cloud_count; j++) {
                weather->clouds[j-1] = weather->clouds[j];
            }
            weather->cloud_count--;
            i--;
        } else {
            
            //individual clouds move with the prevailing wind, and with some slight
            //variation
            c->lifetimelength_subcounter++;
            
            if (c->lifetimelength_subcounter > 2000) {
                c->lifetimelength_subcounter = 0;
                c->cloud_ticks++;
                
                
                if (c->cloud_ticks < c->lifetimelength_subcounter) {
                    int next_state = c->lifetime[c->cloud_ticks];
                    float state_float = next_state/RAND_MAX;
                    
                    if (c->cloud_mode ==  CLOUD_MODE_STABLE) {
                        if (state_float < 0.6) {
                            c->cloud_mode = CLOUD_MODE_STABLE;
                        } else if(state_float < 0.8){
                            if (state_float > 0.78) {
                                c->cloud_mode = CLOUD_MODE_BUILDING_FAST;
                            } else {
                                c->cloud_mode = CLOUD_MODE_BUILDING;
                            }
                        } else if(state_float < 1){
                            if (state_float > 0.98) {
                                c->cloud_mode = CLOUD_MODE_DECAYING_FAST;
                            } else {
                                c->cloud_mode = CLOUD_MODE_DECAYING;
                            }
                        }
                    } else if (c->cloud_mode == CLOUD_MODE_BUILDING || c->cloud_mode ==  CLOUD_MODE_BUILDING_FAST) {
                        if (state_float < 0.4) {
                            c->cloud_mode = CLOUD_MODE_BUILDING;
                        } else if (state_float < 0.6) {
                            c->cloud_mode = CLOUD_MODE_STABLE;
                        } else if (state_float < 0.8) {
                            c->cloud_mode = CLOUD_MODE_BUILDING_FAST;
                        } else if(state_float < 1){
                            if (state_float > 0.98) {
                                c->cloud_mode = CLOUD_MODE_DECAYING_FAST;
                            } else {
                                c->cloud_mode = CLOUD_MODE_DECAYING;
                            }
                        }
                    } else if (c->cloud_mode == CLOUD_MODE_DECAYING || c->cloud_mode ==  CLOUD_MODE_DECAYING_FAST) {
                        if (state_float < 0.2) {
                            c->cloud_mode = CLOUD_MODE_DECAYING;
                        } else if (state_float < 0.7) {
                            c->cloud_mode = CLOUD_MODE_STABLE;
                        } else if (state_float < 0.8) {
                            c->cloud_mode = CLOUD_MODE_DECAYING_FAST;
                        } else if(state_float < 1){
                            if (state_float > 0.98) {
                                c->cloud_mode = CLOUD_MODE_BUILDING_FAST;
                            } else {
                                c->cloud_mode = CLOUD_MODE_BUILDING;
                            }
                        }
                    }
                    
                    
                } else {
                    c->cloud_mode = CLOUD_MODE_DECLINING;
                }
            }
            
            c->x += weather->x_prevailing_winds;
            c->z += weather->z_prevailing_winds;
            
            c->x += ((weather->clouds)[i])->dx;
            c->y += ((weather->clouds)[i])->dy;
            c->z += ((weather->clouds)[i])->dz;
            
            
            if (c->cloud_mode == CLOUD_MODE_BUILDING) {
                c->cloud_life++;
                
                c->sx += 0.01 + (rand() / RAND_MAX) * 0.01;
                c->sy += 0.01 + (rand() / RAND_MAX) * 0.01;
                c->sz += 0.01 + (rand() / RAND_MAX) * 0.01;
                
            } else if (c->cloud_mode == CLOUD_MODE_BUILDING_FAST) {
                c->cloud_life += 2;
                
                c->sx += 0.01 + (rand() / RAND_MAX) * 0.03;
                c->sy += 0.01 + (rand() / RAND_MAX) * 0.03;
                c->sz += 0.01 + (rand() / RAND_MAX) * 0.03;
                
            } else if (c->cloud_mode == CLOUD_MODE_DECAYING) {
                c->cloud_life--;
                c->sx -= 0.01 + (rand() / RAND_MAX) * 0.01;
                c->sy -= 0.01 + (rand() / RAND_MAX) * 0.01;
                c->sz -= 0.01 + (rand() / RAND_MAX) * 0.01;
            } else if (c->cloud_mode == CLOUD_MODE_DECAYING_FAST) {
                c->cloud_life -= 2;
                c->sx -= 0.01 + (rand() / RAND_MAX) * 0.03;
                c->sy -= 0.01 + (rand() / RAND_MAX) * 0.03;
                c->sz -= 0.01 + (rand() / RAND_MAX) * 0.03;
            } else if(c->cloud_mode == CLOUD_MODE_DECLINING){
                
                c->sx -= 0.001 + (rand() / RAND_MAX) * 0.005;
                c->sy -= 0.001 + (rand() / RAND_MAX) * 0.005;
                c->sz -= 0.001 + (rand() / RAND_MAX) * 0.005;
                c->cloud_life = 10000;
            }
            
            if (c->cloud_life <= 0) {
                c->cloud_mode = CLOUD_MODE_DECLINING;
                c->cloud_life = 10000;
            }
            
            if (c->sx < 0 || c->sy < 0 || c->sz < 0) {
                c->cloud_life = 0;
            }
            
            
        }
    }
    
    //add new cloud if required.
    add_cloud(player_x, player_z);
    
}


void add_cloud(float player_x, float player_z){
    //certain types of weather will force less clouds to be allowed.
    int weather_cloud_max_modifier = 0;
    if(weather->cloud_count < MAXIMUM_CLOUDS - weather_cloud_max_modifier){
        Cloud *c = (Cloud*)malloc(sizeof(Cloud));
        c->lifetimelength = 16 + (rand() % 10);
        c->lifetime = malloc(c->lifetimelength * sizeof(int));
        
        
        c->cloud_ticks = 0;
        c->lifetimelength_subcounter = 0;
        
        c->dx = (rand()/RAND_MAX) * 0.01 - 0.005;
        c->dz = (rand()/RAND_MAX) * 0.01 - 0.005;
        
        //randomly distribute around the player
        
        
        c->x = player_x + (rand() % 800) - 400;
        c->y = 0;
        c->z = player_z + (rand() % 800) - 400;
        
        c->r = 0.8f;
        c->g = 0.8f;
        c->b = rand()/RAND_MAX;
        
        c->sx = (rand() % 30) + 10.0f;
        c->sy = (rand() % 4) + 1.0f;
        c->sz = (rand() % 30) + 10.0f;
        
        c->cloud_mode = CLOUD_MODE_STABLE;
        c->cloud_life = 5000;
        
        weather->clouds[weather->cloud_count] = c;
        weather->cloud_count++;
        
        int i;
        for (i=0; i<c->lifetimelength; i++) {
            c->lifetime[i] = rand();
        }

        
        
        printf("[CLOUDS] Added new cloud\n");
    }
}

void render_cloud(Cloud *cloud, CloudAttrib *attrib){
    
    float matrix[16];
    
    float matrix_prev[16];
    glGetUniformfv(attrib->program, attrib->model, matrix_prev);
    glGetUniformfv(attrib->program, attrib->model, matrix);
    
    mat_identity(matrix);
    mat_translate(matrix,cloud->x, 80 + cloud->y, cloud->z);
    
    
    mat_scale(matrix, cloud->sx,cloud->sy,cloud->sz);
    glUniformMatrix4fv(attrib->model, 1, GL_FALSE, matrix);
    
    glUniform3f(attrib->cloudColour, cloud->r, cloud->g, cloud->b);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniformMatrix4fv(attrib->model, 1, GL_FALSE, matrix_prev);
}

void render_clouds(CloudAttrib *attrib,int width, int height, float x, float y, float z, float rx, float ry, float fov, int ortho) {
    int i;


    float matrix[16];
    set_matrix_3d( matrix, width, height, x,y,z,rx,ry, fov, ortho);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, x,y,z);
    glUniform1i(attrib->sampler, 0);
    glUniform1f(attrib->timer, glfwGetTime());
    
    
    
    glBindBuffer(GL_ARRAY_BUFFER, weather->cloud_vertex_buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 8, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 8, (GLvoid *)(sizeof(GLfloat) * 6));
    
    for(i=0; i<weather->cloud_count; i++){
        render_cloud((weather->clouds)[i], attrib);
    }
    

    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->uv);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix_prev);
}

void remove_cloud(Cloud *c){
    free(c->lifetime);
    free(c);
    printf("[CLOUDS] Remove cloud called\n");

}

void cleanup_clouds() {
    printf("[CLOUDS] Cleanup clouds called\n");
    printf("[CLOUDS] Removing %d clouds.\n",weather->cloud_count);
    int i;
    for(i=0; i<weather->cloud_count; i++){
        remove_cloud((weather->clouds)[i]);
    }
    free(weather->clouds);
    
    free(weather);
}
