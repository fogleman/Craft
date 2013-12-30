
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

void set_vertex(GLfloat *d,float x,float y,float z,float nx,float ny,float nz,float r,float g, float b, int *index);

void create_clouds() {
    printf("[CLOUDS] Create clouds called\n");
    weather = (Weather*)malloc(sizeof(Weather));
    
    //set a prevailaing wind direction.
    weather->x_prevailing_winds = 1.0f;
    weather->z_prevailing_winds = 0.0f;
    
    weather->initial_generation = MAXIMUM_CLOUDS;
    
    weather->cloud_count = 0;
    weather->clouds = (Cloud**)malloc(MAXIMUM_CLOUDS * sizeof(Cloud*));
   
    GLfloat *data = malloc(sizeof(GLfloat) * 6 * 9 * 6);
    
    float n = 1.0f;
    int index = 0;

    // -z
    set_vertex(data,-n,n,-n,0,0,-1,1.0,1.0,1.0, &index);
    set_vertex(data, n,n,-n,0,0,-1,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,-n,0,0,-1,1.0,1.0,1.0, &index);
    
    set_vertex(data, n,n,-n,0,0,-1,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n,-n,0,0,-1,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,-n,0,0,-1,1.0,1.0,1.0, &index);
    
    // +z
    set_vertex(data,-n,n,n,0,0,1,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,n,0,0,1,1.0,1.0,1.0, &index);
    set_vertex(data, n,n,n,0,0,1,1.0,1.0,1.0, &index);
    
    set_vertex(data, n,n,n,0,0,1,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,n,0,0,1,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n,n,0,0,1,1.0,1.0,1.0, &index);
    
    // +x
    set_vertex(data,n,n, n,1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n,-n,1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,n,-n,1,0,0,1.0,1.0,1.0, &index);
    
    set_vertex(data,n,-n,-n,1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,n, n,1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n,n,1,0,0,1.0,1.0,1.0, &index);
    
    // -x
    set_vertex(data,-n,n, n,-1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,n,-n,-1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,-n,-1,0,0,1.0,1.0,1.0, &index);
    
    set_vertex(data,-n,n, n,-1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,-n,-1,0,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,n,-1,0,0,1.0,1.0,1.0, &index);
    
    // +y
    set_vertex(data,-n,n,n,0,1,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,n, n,0,1,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,n,-n,0,1,0,1.0,1.0,1.0, &index);
    
    set_vertex(data,n,n, n,0,1,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,n,-n,0,1,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,n,-n,0,1,0,1.0,1.0,1.0, &index);
    
    // -y
    set_vertex(data,-n,-n,-n,0,-1,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n, -n,0,-1,0,1.0,1.0,1.0, &index);
    set_vertex(data,n,-n,n,0,-1,0,1.0,1.0,1.0, &index);
    
    set_vertex(data,n,-n, n,0,-1,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,n,0,-1,0,1.0,1.0,1.0, &index);
    set_vertex(data,-n,-n,-n,0,-1,0,1.0,1.0,1.0, &index);
    
    weather->cloud_vertex_buffer = gen_faces(9, 6, data);
    
}

void set_vertex(GLfloat *d,float x,float y,float z,float nx,float ny,float nz,float r,float g, float b, int *index){
    d[(*index)++] = x; d[(*index)++] = y; d[(*index)++] = z;
    d[(*index)++] = nx; d[(*index)++] = ny; d[(*index)++] = nz;
    d[(*index)++] = r; d[(*index)++] = g; d[(*index)++] = b;
}


void update_clouds(float player_x, float player_z, float rx, float rz) {
    int i;
    for(i=0; i<weather->cloud_count; i++){
        
        Cloud *c = (weather->clouds)[i];
        //delete clouds that have strayed too far from the player, or have degenerated.
        
        if ((pow(player_x - (((weather->clouds)[i])->x),2) + pow(player_z - (((weather->clouds)[i])->z),2)) > pow(500,2)) {
            
            
            remove_cloud(c);
            weather->clouds[i] = NULL;

            
            int j;
            for (j=i+1; j<weather->cloud_count; j++) {
                weather->clouds[j-1] = weather->clouds[j];
            }
            weather->cloud_count--;
            i--;
        } else {
            
            c->x += weather->x_prevailing_winds;
            c->z += weather->z_prevailing_winds;
            
            c->x += ((weather->clouds)[i])->dx;
            c->y += ((weather->clouds)[i])->dy;
            c->z += ((weather->clouds)[i])->dz;
        }
    }
    
    //add new cloud if required.
    add_cloud(player_x, player_z,rx,rz);
    
}

void add_cloud(float player_x, float player_z, float rx, float rz){
    //certain types of weather will force less clouds to be allowed.
    int weather_cloud_max_modifier = 0;
    if(weather->cloud_count < MAXIMUM_CLOUDS - weather_cloud_max_modifier){
        Cloud *c = (Cloud*)malloc(sizeof(Cloud));
        
        c->hmWidth = 32;
        c->hmDepth = 32;
        
        c->heightmap = (int*)calloc(sizeof(int),c->hmWidth * c->hmDepth);
        
        //seed an initial value
        int i,j;
        
        int placed = 0;
        
        while (placed == 0) {
            int x = rand() % 1000;
            int z = rand() % 1000;
            for (i=0; i<c->hmWidth; i++) {
                for (j=0; j<c->hmDepth; j++) {
                    
                    
                    float spx = simplex3( (x+i) * 0.01, 10, (z+j) * 0.01, 8, 0.5, 2);
                    c->heightmap[i * c->hmDepth + j] = (spx > 0.72) ? (int)(((spx - 0.72) / 0.20) * 5) : 0;
                    
                    if (c->heightmap[i * c->hmDepth + j] != 0) {
                        placed = 1;
                    }
                }
            }
        }
        
        c->dx = (rand()/RAND_MAX) * 0.01 - 0.005;
        c->dz = (rand()/RAND_MAX) * 0.01 - 0.005;
        
        //randomly distribute around the player if first time generation
        if (weather->initial_generation > 0) {
            weather->initial_generation--;
            
            c->x = player_x + (rand() % 800) - 400;
            c->z = player_z + (rand() % 800) - 400;
            
        } else {

            if(rand() % 100 < 50){
                c->x = player_x + (rand() % 600) - 300;
                c->z = player_z + ((rand() % 100 < 50) ? 300 : -300);
            } else {
                c->x = player_x + ((rand() % 100 < 50) ? 300 : -300);
                c->z = player_z + (rand() % 600) - 300;
            }
        }
        
        c->y = rand() % 20;
        
        c->sx = (rand() % 10) + 1;
        c->sy = 1;
        c->sz = (rand() % 10) + 1;
        
        
        c->r = 1.0f - (rand()%10)/100;
        c->g = 1.0f - (rand()%10)/100;
        c->b = 1;
        
        
        weather->clouds[weather->cloud_count] = c;
        weather->cloud_count++;
        
    }
}

void render_cloud(Cloud *cloud, CloudAttrib *attrib){
    float matrix[16];
    
    float matrix_prev[16];
    glGetUniformfv(attrib->program, attrib->model, matrix_prev);
    glGetUniformfv(attrib->program, attrib->model, matrix);
    
    mat_identity(matrix);
    
    mat_translate(matrix,cloud->x - (cloud->hmWidth/2), 80 + cloud->y, cloud->z - (cloud->hmDepth/2));
    
    mat_scale(matrix,cloud->sx,cloud->sy,cloud->sz);
    
    glUniform3f(attrib->cloudColour, cloud->r, cloud->g, cloud->b);
   
    int i,j;
    for (i=0; i<cloud->hmWidth; i++) {
        for (j=0; j<cloud->hmDepth; j++) {
            mat_translate_existing(matrix,0,0,1);
            int heightval = cloud->heightmap[i*(cloud->hmDepth) + j];
            if(heightval > 0){
                
                float up = heightval/2.0f;
                mat_translate_existing(matrix,0,up,0);
                mat_scale(matrix, 1,heightval,1);
                glUniformMatrix4fv(attrib->model, 1, GL_FALSE, matrix);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                mat_translate_existing(matrix,0,-up,0);
                mat_scale(matrix, 1,1.0f/heightval,1);
            
            }
        }
        mat_translate_existing(matrix,1,0,-cloud->hmDepth);
    }
    
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
        glUniform1f(attrib->timer, attrib->time);
    
    glUniform1i(attrib->skysampler, 2);
    
    glBindBuffer(GL_ARRAY_BUFFER, weather->cloud_vertex_buffer);
    glEnableVertexAttribArray(attrib->position);
    glEnableVertexAttribArray(attrib->normal);
    
    glEnableVertexAttribArray(attrib->colour);
    glVertexAttribPointer(attrib->position, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 9, 0);
    glVertexAttribPointer(attrib->normal, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 9, (GLvoid *)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(attrib->colour, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 9, (GLvoid *)(sizeof(GLfloat) * 6));

  
    for(i=0; i<weather->cloud_count; i++){
        render_cloud((weather->clouds)[i], attrib);
    }
    

    glDisableVertexAttribArray(attrib->position);
    glDisableVertexAttribArray(attrib->normal);
    glDisableVertexAttribArray(attrib->colour);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
}

void remove_cloud(Cloud *c){
    free(c->heightmap);
    free(c);
}

void cleanup_clouds() {
    
    int i;
    for(i=0; i<weather->cloud_count; i++){
        remove_cloud((weather->clouds)[i]);
    }
    free(weather->clouds);
    
    free(weather);
}
