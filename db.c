#include <stdlib.h>
#include "db.h"
#include "sqlite3.h"

static int db_enabled = 1;
static sqlite3 *db;
static sqlite3_stmt *insert_block_stmt;
static sqlite3_stmt *update_chunk_stmt;
static sqlite3_stmt *get_key_stmt;
static sqlite3_stmt *set_key_stmt;

void db_enable() {
    db_enabled = 1;
}

void db_disable() {
    db_enabled = 0;
}

int get_db_enabled() {
    return db_enabled;
}

int db_init(char *path) {
    if (!db_enabled) {
        return 0;
    }
    static const char *create_query =
        "create table if not exists state ("
        "   x float not null,"
        "   y float not null,"
        "   z float not null,"
        "   rx float not null,"
        "   ry float not null"
        ");"
        "create table if not exists block ("
        "    p int not null,"
        "    q int not null,"
        "    x int not null,"
        "    y int not null,"
        "    z int not null,"
        "    w int not null"
        ");"
        "create table if not exists key ("
        "    p int not null,"
        "    q int not null,"
        "    key int not null"
        ");"
        "create index if not exists block_xyz_idx on block (x, y, z);"
        "create unique index if not exists block_pqxyz_idx on block (p, q, x, y, z);"
        "create unique index if not exists key_pq_idx on key (p, q);";

    static const char *insert_block_query =
        "insert or replace into block (p, q, x, y, z, w) "
        "values (?, ?, ?, ?, ?, ?);";

    static const char *update_chunk_query =
        "select x, y, z, w from block where p = ? and q = ?;";

    static const char *get_key_query =
        "select key from key where p = ? and q = ?;";

    static const char *set_key_query =
        "insert or replace into key (p, q, key) "
        "values (?, ?, ?);";

    int rc;
    rc = sqlite3_open(path, &db);
    if (rc) return rc;
    rc = sqlite3_exec(db, create_query, NULL, NULL, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, insert_block_query, -1, &insert_block_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, update_chunk_query, -1, &update_chunk_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, get_key_query, -1, &get_key_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(db, set_key_query, -1, &set_key_stmt, NULL);
    if (rc) return rc;
    db_begin_transaction();
    return 0;
}

void db_close() {
    if (!db_enabled) {
        return;
    }
    db_commit_transaction();
    sqlite3_finalize(insert_block_stmt);
    sqlite3_finalize(update_chunk_stmt);
    sqlite3_finalize(get_key_stmt);
    sqlite3_finalize(set_key_stmt);
    sqlite3_close(db);
}

void db_begin_transaction() {
    if (!db_enabled) {
        return;
    }
    sqlite3_exec(db, "begin transaction;", NULL, NULL, NULL);
}

void db_commit_transaction() {
    if (!db_enabled) {
        return;
    }
    sqlite3_exec(db, "commit transaction;", NULL, NULL, NULL);
}

void db_commit() {
    if (!db_enabled) {
        return;
    }
    db_commit_transaction();
    db_begin_transaction();
}

void db_save_state(float x, float y, float z, float rx, float ry) {
    if (!db_enabled) {
        return;
    }
    static const char *query =
        "insert into state (x, y, z, rx, ry) values (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    sqlite3_exec(db, "delete from state;", NULL, NULL, NULL);
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    sqlite3_bind_double(stmt, 1, x);
    sqlite3_bind_double(stmt, 2, y);
    sqlite3_bind_double(stmt, 3, z);
    sqlite3_bind_double(stmt, 4, rx);
    sqlite3_bind_double(stmt, 5, ry);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int db_load_state(float *x, float *y, float *z, float *rx, float *ry) {
    if (!db_enabled) {
        return 0;
    }
    static const char *query =
        "select x, y, z, rx, ry from state;";
    int result = 0;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *x = sqlite3_column_double(stmt, 0);
        *y = sqlite3_column_double(stmt, 1);
        *z = sqlite3_column_double(stmt, 2);
        *rx = sqlite3_column_double(stmt, 3);
        *ry = sqlite3_column_double(stmt, 4);
        result = 1;
    }
    sqlite3_finalize(stmt);
    return result;
}

void db_insert_block(int p, int q, int x, int y, int z, int w) {
    if (!db_enabled) {
        return;
    }
    sqlite3_reset(insert_block_stmt);
    sqlite3_bind_int(insert_block_stmt, 1, p);
    sqlite3_bind_int(insert_block_stmt, 2, q);
    sqlite3_bind_int(insert_block_stmt, 3, x);
    sqlite3_bind_int(insert_block_stmt, 4, y);
    sqlite3_bind_int(insert_block_stmt, 5, z);
    sqlite3_bind_int(insert_block_stmt, 6, w);
    sqlite3_step(insert_block_stmt);
}

void db_load_map(Map *map, int p, int q) {
    if (!db_enabled) {
        return;
    }
    sqlite3_reset(update_chunk_stmt);
    sqlite3_bind_int(update_chunk_stmt, 1, p);
    sqlite3_bind_int(update_chunk_stmt, 2, q);
    while (sqlite3_step(update_chunk_stmt) == SQLITE_ROW) {
        int x = sqlite3_column_int(update_chunk_stmt, 0);
        int y = sqlite3_column_int(update_chunk_stmt, 1);
        int z = sqlite3_column_int(update_chunk_stmt, 2);
        int w = sqlite3_column_int(update_chunk_stmt, 3);
        map_set(map, x, y, z, w);
    }
}

int db_get_key(int p, int q) {
    if (!db_enabled) {
        return 0;
    }
    sqlite3_reset(get_key_stmt);
    sqlite3_bind_int(get_key_stmt, 1, p);
    sqlite3_bind_int(get_key_stmt, 2, q);
    if (sqlite3_step(get_key_stmt) == SQLITE_ROW) {
        return sqlite3_column_int(get_key_stmt, 0);
    }
    return 0;
}

void db_set_key(int p, int q, int key) {
    if (!db_enabled) {
        return;
    }
    sqlite3_reset(set_key_stmt);
    sqlite3_bind_int(set_key_stmt, 1, p);
    sqlite3_bind_int(set_key_stmt, 2, q);
    sqlite3_bind_int(set_key_stmt, 3, key);
    sqlite3_step(set_key_stmt);
}
