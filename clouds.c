
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

void getPIP(float *point, float *point_on_plane, float *ray_origin, float *ray_vector){
    float pop_minus_ro[] = {point_on_plane[0] - ray_origin[0],point_on_plane[1] - ray_origin[1],point_on_plane[2] - ray_origin[2]};
    float t = (1 * pop_minus_ro[1])/(1 * ray_vector[1]);
    
    point[0] = ray_origin[0] + (t * ray_vector[0]);
    point[1] = ray_origin[1] + (t * ray_vector[1]);
    point[2] = ray_origin[2] + (t * ray_vector[2]);
    
}


void update_clouds(float player_x, float player_y, float player_z, float rx, float rz, float fov) {
    
    int i;
    rx -= M_PI/2;
    
    float rdx = player_x + (-40 * cos(rx));
    float rdz = player_z + (-40 * sin(rx));
    
    float llx = rx - (fov*(M_PI/180))/1.6;
    float lrx = rx + (fov*(M_PI/180))/1.6;
    
    float ldx = player_x + (200 * cos(llx));
    float ldz = player_z + (200 * sin(llx));
    
    float rhdx = player_x + (200 * cos(lrx));
    float rhdz = player_z + (200 * sin(lrx));
    
    float lvec[] = {ldx - player_x, ldz - player_z};
    float rvec[] = {rhdx - player_x, rhdz - player_z};
    
    float lvec_length = sqrt( lvec[0] * lvec[0] + lvec[1] * lvec[1] );
    float rvec_length = sqrt( rvec[0] * rvec[0] + rvec[1] * rvec[1] );
    
    float lrangle = acos((rvec[0]*lvec[0] + rvec[1]*lvec[1])/(lvec_length * rvec_length));
    
    int rendering = 0;
    
    for(i=0; i<weather->cloud_count; i++){
        
        Cloud *c = (weather->clouds)[i];
        c->render = 0;
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
            
            
            //check if the player is looking down, or if the player is above the clouds,
            //and if the cloud is close enough to not be culled by the frag shader
            if ((rz > -0.444 || player_y > CLOUD_Y_HEIGHT) && (pow(player_x - (((weather->clouds)[i])->x),2) + pow(player_z - (((weather->clouds)[i])->z),2)) < pow(250,2)) {
               
                if (rz > 1.1) {
                        //just check around us
                    if ((pow(player_x - (((weather->clouds)[i])->x),2) + pow(player_z - (((weather->clouds)[i])->z),2)) < pow(130,2)) {
                        c->render = 1;
                        rendering++;
                    }
                } else {
                
                    float cvec[] = {c->x - rdx, c->z - rdz};
                    
                    float angle = acos((cvec[0]*lvec[0] + cvec[1]*lvec[1])/(sqrt(pow(cvec[0],2)+pow(cvec[1],2))*sqrt(pow(lvec[0],2)+pow(lvec[1],2))));
                    
                    cvec[0] = c->x + c->hmWidth - rdx;
                    cvec[1] = c->z + c->hmDepth - rdx;

                    float angletwo = acos((cvec[0]*lvec[0] + cvec[1]*lvec[1])/(sqrt(pow(cvec[0],2)+pow(cvec[1],2))*sqrt(pow(lvec[0],2)+pow(lvec[1],2))));
                    
                    
                    if (angle < lrangle || angletwo < lrangle) {
                        c->render = 1;
                        rendering++;
                    }
                }
                
            }

  
        }
    }
    
    //printf("Going to render %d/%d\n",rendering,weather->cloud_count);
    
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
        c->render = 0;
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
    
    mat_translate(matrix,cloud->x - (cloud->hmWidth/2), CLOUD_Y_HEIGHT + cloud->y, cloud->z - (cloud->hmDepth/2));
    
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
    if(!(ry > -0.444 || y > CLOUD_Y_HEIGHT)){
        return;
    }

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
        if ((weather->clouds)[i]->render) {
            render_cloud((weather->clouds)[i], attrib);
        }
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
