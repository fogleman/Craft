#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "konstructs.h"
#include "inventory.h"

Inventory inventory;
Inventory ext_inventory;
Model *g;

void render_belt_block(Attrib *attrib, int pos, Item block) {

    float scale = 0.7;      // block scale
    float s = 0.15 * WINDOW_WIDTH/g->width;
    float xpos = (s * INVENTORY_SLOTS)/-2 + s/2 + s*pos;
    float ypos = ((float)g->height - 120.0f)/(float)g->height;
    int sel = inventory.selected == INVENTORY_SLOTS - pos - 1 ? 1 : 0;

    glUseProgram(attrib->program);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0); // GL_TEXTURE0
    glUniform1f(attrib->timer, PI*2);
    float matrix[16];
    GLuint buffer;
    set_matrix_item_offs(matrix, g->width, g->height, scale, xpos, ypos, sel);
    if (is_plant(block.id)) {
        glDeleteBuffers(1, &buffer);
        buffer = gen_plant_buffer(0, 0, 0, 0.5, block.id);
    } else {
        buffer = gen_cube_buffer(0, 0, 0, 0.5, block.id);
    }
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    if (is_plant(block.id)) {
        draw_plant(attrib, buffer);
    } else {
        draw_cube(attrib, buffer);
    }
    del_buffer(buffer);

}

void render_ext_inventory_block(Attrib *attrib, int row, int col, Item block) {

    float scale = 0.5;      // block scale
    float s = 0.12 * WINDOW_WIDTH/g->width;
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float xpos = (s * EXT_INVENTORY_COLS)/-2 + s/2 + s*col;
    float ypos = ((float)g->height - 260.0f)/(float)g->height - row * v - v/3;
    int sel = 1;

    glUseProgram(attrib->program);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0); // GL_TEXTURE0
    glUniform1f(attrib->timer, PI*2);
    glUniform4f(attrib->extra5,0.0, 0.0, 0.0, 0.0);
    float matrix[16];
    GLuint buffer;
    set_matrix_item_offs(matrix, g->width, g->height, scale, xpos, ypos, sel);
    if (is_plant(block.id)) {
        glDeleteBuffers(1, &buffer);
        buffer = gen_plant_buffer(0, 0, 0, 0.5, block.id);
    } else {
        buffer = gen_cube_buffer(0, 0, 0, 0.5, block.id);
    }
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    if (is_plant(block.id)) {
        draw_plant(attrib, buffer);
    } else {
        draw_cube(attrib, buffer);
    }
    del_buffer(buffer);
}

void render_inventory_items(Attrib *attrib, float xoffs, float yoffs, float scale, int ioffs,
        int width, int height) {
    for (int item = ioffs; item < ioffs+INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item];
        if (block.id == 0 || block.num == 0) continue;

        float slotoff = -1 *  ((float)item - (float)(INVENTORY_SLOTS - 1) / 2);
        float xpos = slotoff * ((0.125*scale*1024)/width) + xoffs;

        int sel = inventory.selected == item ? 1 : 0;
        render_inventory_item(attrib, block, xpos, yoffs, scale, width, height, sel);
    }
}

void render_belt_texts(Attrib *attrib) {

    for (int item = 0; item < INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item];
        if (block.id == 0 || block.num <= 0) continue;
        render_belt_text(attrib, item, block);
    }

}

void render_belt_text(Attrib *attrib, int pos, Item block) {

    float s = 0.15 * WINDOW_WIDTH/g->width;
    float gl_x = (s * INVENTORY_SLOTS)/-2 + s/2 + s*pos;
    float gl_y = -1 * ((float)g->height - 70.0f)/(float)g->height;

    int x = (g->width / 2)  + (g->width / 2)  * gl_x;
    int y = (g->height / 2) + (g->height / 2) * gl_y;

    float matrix[16];
    mat_ortho(matrix, 0, g->width, 0, g->height, -1, 10);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1); // GL_TEXTURE1
    char text_buffer[16];
    snprintf(text_buffer, 16, "%02d", block.num);
    print(attrib, 1, x, y, 12, text_buffer);
}

void render_ext_inventory_text(Attrib *attrib, int row, int col, Item block) {

    float s = 0.12 * WINDOW_WIDTH/g->width;
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float gl_x = (s * EXT_INVENTORY_COLS)/-2 + s/2 + s*col;
    float gl_y = (-1 * ((float)g->height - 260.0f)/(float)g->height) + row * v;

    int x = (gl_x * g->width) / 2 + (g->width / 2);
    int y = (gl_y * g->height) / 2 + (g->height / 2);

    float matrix[16];
    mat_ortho(matrix, 0, g->width, 0, g->height, -1, 10);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1); // GL_TEXTURE1
    char text_buffer[16];
    snprintf(text_buffer, 16, "%02d", block.num);
    print(attrib, 1, x, y, 12, text_buffer);
}

void render_belt_text_blocks(Attrib *text_attrib, Attrib *block_attrib) {
    for (int item = 0; item < INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item];
        if (block.id == 0 || block.num <= 0) continue;
        render_belt_text(text_attrib, item, block);
        render_belt_block(block_attrib, INVENTORY_SLOTS - item - 1, block);
    }
}

void render_ext_inventory_text_blocks(Attrib *text_attrib, Attrib *block_attrib) {
    for (int i = 0; i < EXT_INVENTORY_ROWS; i ++) {
        for (int j = 0; j < EXT_INVENTORY_COLS; j ++) {
            Item block = ext_inventory.items[i*EXT_INVENTORY_COLS + j];
            if (block.id == 0 || block.num <= 0) continue;
            render_ext_inventory_block(block_attrib, i, EXT_INVENTORY_COLS - j - 1, block);
            render_ext_inventory_text(text_attrib, i, j, block);
        }
    }
}

/*
 * Render the inventory belt on the bottom center of the screen.
 */
void render_belt_background(Attrib *attrib, int selected) {

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLfloat vertex_data[INVENTORY_SLOTS * 6 * 4];

    float s = 0.15 * WINDOW_WIDTH/g->width; // belt size on screen
    float px = (s*INVENTORY_SLOTS)/-2 + s;  // belt start x
    float py = -1 * ((float)g->height - 50.0f)/(float)g->height;  // belt pos y
    float t = 0;                            // selected default image
    float ts = 0.25;                        // image size (1/images)
    int lt = t;                             // image to show

    // Generate matrix for all inventory belt slots
    for (int i=0; i<INVENTORY_SLOTS; i++) {

        lt = selected == i ? t + 1 : t;

        GLfloat side[] = {
        //   X            Y      U           V
             (i*s)+px-s,  py+s,  (ts*lt),    1.0f,
             (i*s)+px,    py,    (ts*lt)+ts, 0.0f,
             (i*s)+px-s,  py,    (ts*lt),    0.0f,

             (i*s)+px,    py+s,  (ts*lt)+ts, 1.0f,
             (i*s)+px,    py,    (ts*lt)+ts, 0.0f,
             (i*s)+px-s,  py+s,  (ts*lt),    1.0f,
        };
        memcpy(vertex_data + 6*4*i, side, sizeof(float)*6*4);
    }

    // Load vertex_data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glUseProgram(attrib->program);

    // Our texture is saved in GL_TEXTURE4 from main.c
    glUniform1i(attrib->sampler, 4);

    glEnableVertexAttribArray(attrib->position);
    glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          0);

    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          (void*)(2*sizeof(float)));

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, INVENTORY_SLOTS * 6);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

}

void render_ext_inventory_background(Attrib *attrib) {

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLfloat vertex_data[EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS * 6 * 4];

    float s = 0.12 * WINDOW_WIDTH/g->width;   // belt size on screen
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float px = (s*EXT_INVENTORY_COLS)/-2 + s;  // belt start x
    float py = -1 * ((float)g->height - 250.0f)/(float)g->height;  // belt pos y
    float t = 2;                            // selected default image
    float ts = 0.25;                        // image size (1/images)
    int lt = t;                             // image to show

    // Generate matrix for all inventory belt slots
    for (int i=0; i<EXT_INVENTORY_ROWS; i++) {
        for (int j=0; j<EXT_INVENTORY_COLS; j++) {
            GLfloat side[] = {
            //   X           Y            U           V
                (j*s)+px-s,  (i*v)+py+v,  (ts*lt),    1.0f,
                (j*s)+px,    (i*v)+py,    (ts*lt)+ts, 0.0f,
                (j*s)+px-s,  (i*v)+py,    (ts*lt),    0.0f,

                (j*s)+px,    (i*v)+py+v,  (ts*lt)+ts, 1.0f,
                (j*s)+px,    (i*v)+py,    (ts*lt)+ts, 0.0f,
                (j*s)+px-s,  (i*v)+py+v,  (ts*lt),    1.0f,
            };
            memcpy(vertex_data + 6*4 * (i*EXT_INVENTORY_COLS + j),
                   side, sizeof(float)*6*4);
        }
    }

    // Load vertex_data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    glUseProgram(attrib->program);

    // Our texture is saved in GL_TEXTURE4 from main.c
    glUniform1i(attrib->sampler, 4);

    glEnableVertexAttribArray(attrib->position);
    glVertexAttribPointer(attrib->position, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          0);

    glEnableVertexAttribArray(attrib->uv);
    glVertexAttribPointer(attrib->uv, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(GLfloat),
                          (void*)(2*sizeof(float)));

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS * 6);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

}

