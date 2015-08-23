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

// Render a cube with ID w at x, y. This function is used to render the inventory
// blocks, like the belt and the inventory screen.
void render_inventory_block(Attrib *attrib, int w, float s, float x, float y, int flag) {
    glUseProgram(attrib->program);
    glUniform3f(attrib->camera, 0, 0, 5);
    glUniform1i(attrib->sampler, 0); // GL_TEXTURE0
    glUniform1f(attrib->timer, PI*2);
    float identity[16];
    mat_identity(identity);
    glUniformMatrix4fv(attrib->extra5, 1, GL_FALSE, identity);
    float matrix[16];
    GLuint buffer;

    // Default block rotations
    float rx = -PI/4;
    float ry = -PI/10;
    float rz = 0;
    float dz = 0;

    switch (flag) {
        case 1:
            rx = -PI/8;
            break;
        case 2:
            rx = -PI/8;
            ry = -PI/10;
            rz = -PI/16;
            dz = -1.0;
            break;
        case 3:
            rx = -PI/8;
            ry = -PI/10;
            rz = -PI/16;
            dz = 2.0;
            break;
    }

    set_matrix_item_r(matrix, g->width, g->height, s, x, y, rx, ry, rz);

    if (is_plant(w)) {
        glDeleteBuffers(1, &buffer);
        buffer = gen_plant_buffer(0, 0, dz, 0.5, w);
    } else {
        buffer = gen_cube_buffer(0, 0, dz, 0.5, w);
    }
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    if (is_plant(w)) {
        draw_plant(attrib, buffer);
    } else {
        draw_cube(attrib, buffer);
    }
    del_buffer(buffer);
}

// Render a block under the mouse pointer
void render_mouse_block(Attrib *attrib) {

    // Do not render air, or if the feature is disabled.
    if (g->mouse_item <= 0) return;

    double xpos, ypos;
    double size = 32;

    glfwGetCursorPos(g->window, &xpos, &ypos);

    float gl_x = ((xpos - size)/g->width * 2 - 1) * -1;
    float gl_y = ((ypos - size/2)/g->height * 2 - 1);

    render_inventory_block(attrib, g->mouse_item, 0.58, gl_x, gl_y, 3);
}

// Render a block in the hand
void render_hand_blocks(Attrib *attrib) {
    int w = inventory.items[inventory.selected].id;
    if (w > 0) {
        render_inventory_block(attrib, w, 4.0, -1.2, 0.97, 2);
    }
}

// Render a block at the belt at position pos.
void render_belt_block(Attrib *attrib, int pos, Item block) {

    float scale = 0.7;      // block scale
    float s = 0.15 * WINDOW_WIDTH/g->width;
    float xpos = (s * INVENTORY_SLOTS)/-2 + s/2 + s*pos;
    float ypos = ((float)g->height - 120.0f)/(float)g->height;
    int sel = inventory.selected == INVENTORY_SLOTS - pos - 1 ? 1 : 0;

    render_inventory_block(attrib, block.id, scale, xpos, ypos, sel);
}

// Render a block in the inventory at col and row.
void render_ext_inventory_block(Attrib *attrib, int row, int col, Item block) {

    float scale = 0.5;      // block scale
    float s = 0.12 * WINDOW_WIDTH/g->width;
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float xpos = (s * EXT_INVENTORY_COLS)/-2 + s/2 + s*col;
    float ypos = ((float)g->height
                 - EXT_INVENTORY_PX_FROM_BOTTOM - 10.0f)/(float)g->height - row * v - v/3;
    int sel = 1;

    render_inventory_block(attrib, block.id, scale, xpos, ypos, sel);
}

// Render a text displaying a int at x, y (opengl coords)
void render_inventory_number_at(Attrib *attrib, int num, float x, float y) {
    int nx = (g->width / 2)  + (g->width / 2)  * x;
    int ny = (g->height / 2) + (g->height / 2) * y;

    float matrix[16];
    mat_ortho(matrix, 0, g->width, 0, g->height, -1, 10);
    glUseProgram(attrib->program);
    glUniformMatrix4fv(attrib->matrix, 1, GL_FALSE, matrix);
    glUniform1i(attrib->sampler, 1); // GL_TEXTURE1
    char text_buffer[16];
    snprintf(text_buffer, 16, "%02d", num);
    print(attrib, 1, nx, ny, 12, text_buffer);
}

// Render the text at the belt.
void render_belt_text(Attrib *attrib, int pos, Item block) {

    float s = 0.15 * WINDOW_WIDTH/g->width;
    float x = (s * INVENTORY_SLOTS)/-2 + s/2 + s*pos;
    float y = -1 * ((float)g->height - 70.0f)/(float)g->height;

    render_inventory_number_at(attrib, block.num, x, y);
}

// Render the text in the inventory.
void render_ext_inventory_text(Attrib *attrib, int row, int col, Item block) {

    float s = 0.12 * WINDOW_WIDTH/g->width;
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float x = (s * EXT_INVENTORY_COLS)/-2 + s/2 + s*col;
    float y = (-1 * ((float)g->height
              - EXT_INVENTORY_PX_FROM_BOTTOM - 10.0f)/(float)g->height) + row * v;

    render_inventory_number_at(attrib, block.num, x, y);
}

// Called from main loop, renders the belt's blocks and texts.
void render_belt_text_blocks(Attrib *text_attrib, Attrib *block_attrib) {
    for (int item = 0; item < INVENTORY_SLOTS; item ++) {
        Item block = inventory.items[item];
        if (block.id == 0 || block.num <= 0) continue;
        render_belt_text(text_attrib, item, block);
        render_belt_block(block_attrib, INVENTORY_SLOTS - item - 1, block);
    }
}

// Called from main loop, renders the inventory blocks and texts.
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

// Prep a vertex array and vertex buffer object.
void prep_2dtexture_buffers(GLuint *vao, GLuint *vbo) {
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
}

// Ask the graphic card to render the already provided buffer.
void render_2dtexture(Attrib *attrib, int num) {
    glUseProgram(attrib->program);
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
    glDrawArrays(GL_TRIANGLES, 0, num);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

// Render the 2d texture background for our belt.
void render_belt_background(Attrib *attrib, int selected) {

    GLuint vao, vbo;
    prep_2dtexture_buffers(&vao, &vbo);

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

    render_2dtexture(attrib, INVENTORY_SLOTS * 6);
}

// Render the 2d texture background for the inventory.
void render_ext_inventory_background(Attrib *attrib) {

    GLuint vao, vbo;
    prep_2dtexture_buffers(&vao, &vbo);

    GLfloat vertex_data[EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS * 6 * 4];

    float s = 0.12 * WINDOW_WIDTH/g->width;   // belt size on screen
    float v = 0.12 * WINDOW_HEIGHT/g->height;
    float px = (s*EXT_INVENTORY_COLS)/-2 + s;  // belt start x
    float py = -1 * ((float)g->height
               - EXT_INVENTORY_PX_FROM_BOTTOM)/(float)g->height;  // belt pos y
    float t = 2;                            // selected default image
    float ts = 0.25;                        // image size (1/images)
    int lt = t;                             // image to show

    // Generate matrix for all inventory belt slots
    for (int i=0; i<EXT_INVENTORY_ROWS; i++) {
        for (int j=0; j<EXT_INVENTORY_COLS; j++) {
            if (ext_inventory.items[i*EXT_INVENTORY_COLS + j].show == 0) {
                GLfloat side[] = {
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f
                };

                memcpy(vertex_data + 6*4 * (i*EXT_INVENTORY_COLS + j),
                    side, sizeof(float)*6*4);
            } else {

                // Select the next texture if, this slot is selected
                if (ext_inventory.selected == i*EXT_INVENTORY_COLS + j) {
                    lt = t + 1;
                } else {
                    lt = t;
                }

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
    }

    // Load vertex_data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    render_2dtexture(attrib, EXT_INVENTORY_COLS * EXT_INVENTORY_ROWS * 6);
}
