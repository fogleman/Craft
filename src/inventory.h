#ifndef _inventory_h_
#define _inventory_h_

#include <stdint.h>
#include "item.h"

typedef struct {
    #define INVENTORY_COUNT_LEN (Item_max + 1)
    uint8_t count[INVENTORY_COUNT_LEN];
} Inventory;

void Inventory_reset(Inventory *m);
int Inventory_collect(Inventory *m, int w);
int Inventory_use(Inventory *m, int w);
uint8_t Inventory_getCount(Inventory *m, int w);

#endif // _inventory_h_
