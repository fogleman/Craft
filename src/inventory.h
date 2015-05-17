#ifndef _inventory_h_
#define _inventory_h_

#include "konstructs.h"

typedef struct {
    int id;
    int num;
} Item;

typedef struct {
    Item *items;
    int selected;
} Inventory;

void make_inventory(float *data, float x, float y, float n, float m, int s);
GLuint gen_inventory_buffers(float x, float y, float n, int sel);
void draw_inventory(Attrib *attrib, GLuint buffer, int length);
void render_inventory_item(Attrib *attrib, Item item, float xpos, float ypos, float scale,
		int width, int height, int sel);
void render_inventory_items(Attrib *attrib, float xoffs, float yoffs, float scale, int ioffs,
		int width, int height);
void render_inventory_bar(Attrib *attrib, float x, float y, float scale, int sel, int width, int height);
void render_inventory_text(Attrib *attrib, Item item, float x, float y, float scale, int width, int height);
void render_inventory_texts(Attrib *attrib, float x, float y, float scale, int ioffs, int width, int height);
void render_inventory(Attrib *window_attrib, Attrib *block_attrib, Attrib *text_attrib,
	float xoffs, float yoffs, float scale, int sel, int row, int width, int height);

extern Inventory inventory;

#endif
