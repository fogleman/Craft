#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "konstructs.h"
#include "inventory.h"

Inventory inventory;
Inventory ext_inventory;

// From https://github.com/CouleeApps/Craft/tree/mining_crafting
void make_inventory(float *data, float x, float y, float n, float m, int s) {
    float *d = data;
    float z = 0.5;
    float a = z;
    float b = z * 2;
    int w = s;
    float du = w * a;
    float p = 0;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = p;
    *(d++) = x + n; *(d++) = y - m;
    *(d++) = du + a; *(d++) = p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = b - p;
    *(d++) = x - n; *(d++) = y - m;
    *(d++) = du + 0; *(d++) = p;
    *(d++) = x + n; *(d++) = y + m;
    *(d++) = du + a; *(d++) = b - p;
    *(d++) = x - n; *(d++) = y + m;
    *(d++) = du + 0; *(d++) = b - p;
}

// From https://github.com/CouleeApps/Craft/tree/mining_crafting
GLuint gen_inventory_buffers(float x, float y, float n, int sel) {
    int length = INVENTORY_SLOTS;
    GLfloat *data = malloc_faces(4, length);
    x -= n * (length - 1) / 2;
    for (int i = 0; i < length; i ++) {
        make_inventory(data + i * 24, x, y, n / 2, n / 2, sel == i ? 1 : 0);
        x += n;
    }
    return gen_faces(4, length, data);
}

// From https://github.com/CouleeApps/Craft/tree/mining_crafting
void draw_inventory(Attrib *attrib, GLuint buffer, int length) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_triangles_2d(attrib, buffer, length * 6);
    glDisable(GL_BLEND);
}

// Modified version from https://github.com/CouleeApps/Craft/tree/mining_crafting
void render_inventory_item(Attrib *attrib, Item item, float xpos, float ypos, float scale,
        int width, int height, int sel) {
    glUseProgram(attrib->program);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0); // GL_TEXTURE0
    glUniform1f(attrib->timer, PI*2);
    glUniform4f(attrib->extra5,0.0, 0.0, 0.0, 0.0);
    float matrix[16];
    GLuint buffer;
    set_matrix_item_offs(matrix, width, height, 0.65 * scale, xpos, ypos, sel);
    if (is_plant(item.id)) {
        glDeleteBuffers(1, &buffer);
        buffer = gen_plant_buffer(0, 0, 0, 0.5, item.id);
    } else {
        buffer = gen_cube_buffer(0, 0, 0, 0.5, item.id);
    }
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    float identity[16];
    mat_identity(identity);
    glUniformMatrix4fv(attrib->extra6, 1, GL_FALSE, identity);
    if (is_plant(item.id)) {
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

// Modified version from https://github.com/CouleeApps/Craft/tree/mining_crafting
void render_inventory_bar(Attrib *attrib, float x, float y, float scale, int sel,
        int width, int height) {
    float matrix[16];
    set_matrix_2d(matrix, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 4); // GL_TEXTURE4
    GLuint inv_buffer = gen_inventory_buffers((width/2)*(x*-1+1), (height/2)*(y*-1+1), 64*scale, sel);
    draw_inventory(attrib, inv_buffer, INVENTORY_SLOTS);
    del_buffer(inv_buffer);
}

// Modified version from https://github.com/CouleeApps/Craft/tree/mining_crafting
void render_inventory_text(Attrib *attrib, Item item, float x, float y, float scale,
        int width, int height) {
    float matrix[16];
    set_matrix_2d(matrix, width, height);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1); // GL_TEXTURE1
    char text_buffer[16];
    float ts = 12*scale;
    snprintf(text_buffer, 16, "%02d", item.num);
    x += ts * strlen(text_buffer);
    print(attrib, 1, x, y, ts, text_buffer);
}

void render_inventory_texts(Attrib *attrib, float x, float y, float scale, int ioffs, int width, int height) {
    for (int item = ioffs; item < ioffs + INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item];
        if (block.id == 0 || block.num <= 0) continue;

        float tx = (width/2)*(x*-1+1) - (INVENTORY_SLOTS * 64*scale)/2 + (item * 64*scale) + 12*scale;
        float ty = (height/2)*(y*-1+1) - 32*scale;
        render_inventory_text(attrib, block, tx, ty, scale, width, height);
    }
}

void render_inventory(Attrib *window_attrib, Attrib *block_attrib, Attrib *text_attrib,
        float xoffs, float yoffs, float scale, int sel, int row, int width, int height) {
    int ioffs = INVENTORY_SLOTS * row;
    render_inventory_bar(window_attrib, xoffs, yoffs, scale, sel, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_inventory_items(block_attrib, xoffs, yoffs, scale, ioffs, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    render_inventory_texts(text_attrib, xoffs, yoffs, scale, ioffs, width, height);
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

    float s = 0.15;           // belt size on screen
    float px = (s*9)/-2 + s;  // belt start x
    float py = -0.9;          // belt pos y
    float t = 1;              // selected default image
    float ts = 0.25;          // image size (1/images)
    int lt = t;               // image to show

    // Generate matrix for all inventory belt slots
    for (int i=0; i<INVENTORY_SLOTS; i++) {

        lt = selected == i ? t + 1 : t;

        GLfloat side[] = {
        //   X            Y      U           V
             (i*s)+px-s,  py+s,  (ts*lt),    1.0f,
             (i*s)+px,    py,    (ts*lt)+ts, 0.0f,
             (i*s)+px-s,  py,    (ts*lt),    1.0f,

             (i*s)+px,    py+s,  (ts*lt)+ts, 0.0f,
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
    glDrawArrays(GL_TRIANGLES, 0, INVENTORY_SLOTS * 6);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

}


