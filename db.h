#ifndef _db_h_
#define _db_h_

#include "map.h"

#define DB_NAME "mine.db"

int db_init();
void db_close();
void db_insert(int p, int q, int x, int y, int z, int w);
int db_select(int x, int y, int z);
void db_apply(Map *map, int p, int q);

#endif
