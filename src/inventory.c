#include "inventory.h"

void Inventory_reset(Inventory *m) {
    for(int i = 0; i < INVENTORY_COUNT_LEN; i++) {
        m->count[i] = 0;
    }
}

int Inventory_collect(Inventory *m, int w) {
    if(m->count[w] < 255) {
        m->count[w]++;
        return 1;
    } else {
        return 0;
    }
}

int Inventory_use(Inventory *m, int w) {
    if(m->count[w]) {
        m->count[w]--;
        return 1;
    } else {
        return 0;
    }
}

uint8_t Inventory_getCount(Inventory *m, int w) {
    return m->count[w];
}
