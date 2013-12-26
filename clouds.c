
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



void update_clouds(Player *player) {


    int i;
    for(i=0; i<weather->cloud_count; i++){
        
        //individual clouds move with the prevailing wind, and with some slight
        //variation
        
        ((weather->clouds)[i])->x += weather->x_prevailing_winds;
        ((weather->clouds)[i])->z += weather->z_prevailing_winds;
        
        ((weather->clouds)[i])->x += ((weather->clouds)[i])->dx;
        ((weather->clouds)[i])->y += ((weather->clouds)[i])->dy;
        ((weather->clouds)[i])->z += ((weather->clouds)[i])->dz;    }
    
    //add new cloud if required.
    add_cloud(player);
    
}


void add_cloud(Player *player){
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
        State *s = &player->state;
        
        c->x = s->x + (rand() % 400) - 200;
        c->y = s->y + 0;
        c->z = s->z + (rand() % 400) - 200;
        
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

void render_cloud(Cloud *cloud, Attrib *attrib){
    
    float matrix[16];
    
    float matrix_prev[16];
    glGetUniformfv(attrib->program, attrib->model, matrix_prev);
    glGetUniformfv(attrib->program, attrib->model, matrix);
    
    mat_identity(matrix);
    mat_translate(matrix,cloud->x, 80 + cloud->y, cloud->z);
    
    
    mat_scale(matrix, cloud->sx,cloud->sy,cloud->sz);
    glUniformMatrix4fv(attrib->model, 1, GL_FALSE, matrix);
    
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glUniformMatrix4fv(attrib->model, 1, GL_FALSE, matrix_prev);
}

void render_clouds(Attrib *attrib,int width, int height, Player *player, float fov, int ortho) {
    int i;

    State *s = &player->state;

    float matrix[16];
    set_matrix_3d( matrix, width, height, s->x, s->y, s->z, s->rx, s->ry, fov, ortho);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform3f(attrib->camera, s->x, s->y, s->z);
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
