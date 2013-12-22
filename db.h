#ifndef _db_h_
#define _db_h_

#include "map.h"

void db_enable();
void db_disable();
int get_db_enabled();
int db_init(char *path);
void db_close();
int db_worker_run(void *arg);
void db_worker_start(char *path);
void db_worker_stop();
void db_begin_transaction();
void db_commit_transaction();
void db_commit();
void db_save_state(float x, float y, float z, float rx, float ry);
int db_load_state(float *x, float *y, float *z, float *rx, float *ry);
void db_insert_block(int p, int q, int x, int y, int z, int w);
void _db_insert_block(int p, int q, int x, int y, int z, int w);
void db_load_map(Map *map, int p, int q);
int db_get_key(int p, int q);
void db_set_key(int p, int q, int key);
void _db_set_key(int p, int q, int key);

#endif
