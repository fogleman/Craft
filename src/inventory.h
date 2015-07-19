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

void render_belt_background(Attrib *inventory_attrib, int selected);
void render_belt_text_blocks(Attrib *text_attrib, Attrib *block_attrib);

extern Inventory inventory;
extern Inventory ext_inventory;

#endif
