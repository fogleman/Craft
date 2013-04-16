#include <stdlib.h>
#include "db.h"
#include "sqlite3.h"

static sqlite3 *db;
static sqlite3_stmt *insert_stmt;
static sqlite3_stmt *select_stmt;

static const char *create_query =
    "create table if not exists block ("
    "    p int not null,"
    "    q int not null,"
    "    x int not null,"
    "    y int not null,"
    "    z int not null,"
    "    w int not null"
    ");"
    "create index if not exists block_pq_idx on block(p, q);"
    "create unique index if not exists block_xyz_idx on block (x, y, z);";

static const char *insert_query =
    "insert or replace into block (p, q, x, y, z, w) "
    "values (?, ?, ?, ?, ?, ?);";

static const char *select_query =
    "select x, y, z, w from block where p = ? and q = ?;";

int db_init() {
    int rc;
    rc = sqlite3_open(DB_NAME, &db);
    if (rc) return rc;
    rc = sqlite3_exec(db, create_query, NULL, NULL, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, insert_query, -1, &insert_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, select_query, -1, &select_stmt, NULL);
    if (rc) return rc;
    return 0;
}

void db_close() {
    sqlite3_finalize(insert_stmt);
    sqlite3_finalize(select_stmt);
    sqlite3_close(db);
}

void db_insert(int p, int q, int x, int y, int z, int w) {
    sqlite3_reset(insert_stmt);
    sqlite3_bind_int(insert_stmt, 1, p);
    sqlite3_bind_int(insert_stmt, 2, q);
    sqlite3_bind_int(insert_stmt, 3, x);
    sqlite3_bind_int(insert_stmt, 4, y);
    sqlite3_bind_int(insert_stmt, 5, z);
    sqlite3_bind_int(insert_stmt, 6, w);
    sqlite3_step(insert_stmt);
}

void db_apply(Map *map, int p, int q) {
    sqlite3_reset(select_stmt);
    sqlite3_bind_int(select_stmt, 1, p);
    sqlite3_bind_int(select_stmt, 2, q);
    while (sqlite3_step(select_stmt) == SQLITE_ROW) {
        int x = sqlite3_column_int(select_stmt, 0);
        int y = sqlite3_column_int(select_stmt, 1);
        int z = sqlite3_column_int(select_stmt, 2);
        int w = sqlite3_column_int(select_stmt, 3);
        map_set(map, x, y, z, w);
    }
}
