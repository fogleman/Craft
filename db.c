#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "db.h"
#include "ring.h"
#include "sqlite3.h"
#include "tinycthread.h"

static int db_enabled = 0;

static sqlite3 *reader;
static sqlite3_stmt *load_map_stmt;
static sqlite3_stmt *get_key_stmt;

static sqlite3 *writer;
static sqlite3_stmt *insert_block_stmt;
static sqlite3_stmt *set_key_stmt;

static Ring ring;
static thrd_t writer_thread;
static mtx_t mtx;
static cnd_t cnd;

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
    static const char *load_map_query =
        "select x, y, z, w from block where p = ? and q = ?;";
    static const char *get_key_query =
        "select key from key where p = ? and q = ?;";
    int rc;
    rc = sqlite3_open(path, &reader);
    if (rc) return rc;
    rc = sqlite3_exec(reader, create_query, NULL, NULL, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(reader, load_map_query, -1, &load_map_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(reader, get_key_query, -1, &get_key_stmt, NULL);
    if (rc) return rc;
    db_writer_start(path);
    return 0;
}

void db_close() {
    if (!db_enabled) {
        return;
    }
    sqlite3_finalize(load_map_stmt);
    sqlite3_finalize(get_key_stmt);
    sqlite3_close(reader);
    db_writer_stop();
}

int db_writer_run(void *arg) {
    char *path = (char *)arg;
    static const char *insert_block_query =
        "insert or replace into block (p, q, x, y, z, w) "
        "values (?, ?, ?, ?, ?, ?);";
    static const char *set_key_query =
        "insert or replace into key (p, q, key) "
        "values (?, ?, ?);";
    int rc;
    rc = sqlite3_open(path, &writer);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(
        writer, insert_block_query, -1, &insert_block_stmt, NULL);
    if (rc) return rc;
    rc = sqlite3_prepare_v2(
        writer, set_key_query, -1, &set_key_stmt, NULL);
    if (rc) return rc;
    db_begin_transaction();
    time_t last_commit = time(NULL);
    while (1) {
        int p, q, x, y, z, w, key;
        mtx_lock(&mtx);
        while (!ring_get(&ring, &p, &q, &x, &y, &z, &w, &key)) {
            cnd_wait(&cnd, &mtx);
        }
        mtx_unlock(&mtx);
        if (key < 0) {
            break;
        }
        else if (key) {
            _db_set_key(p, q, key);
        }
        else {
            _db_insert_block(p, q, x, y, z, w);
        }
        time_t now = time(NULL);
        if (now - last_commit >= 5) {
            last_commit = now;
            db_commit();
        }
    }
    db_commit_transaction();
    sqlite3_finalize(insert_block_stmt);
    sqlite3_finalize(set_key_stmt);
    sqlite3_close(writer);
    return 0;
}

void db_writer_start(char *path) {
    if (!db_enabled) {
        return;
    }
    ring_alloc(&ring, 1024);
    mtx_init(&mtx, mtx_plain);
    cnd_init(&cnd);
    thrd_create(&writer_thread, db_writer_run, path);
}

void db_writer_stop() {
    if (!db_enabled) {
        return;
    }
    mtx_lock(&mtx);
    ring_put(&ring, 0, 0, 0, 0, 0, 0, -1);
    cnd_signal(&cnd);
    mtx_unlock(&mtx);
    thrd_join(writer_thread, NULL);
    cnd_destroy(&cnd);
    mtx_destroy(&mtx);
    ring_free(&ring);
}

void db_begin_transaction() {
    if (!db_enabled) {
        return;
    }
    sqlite3_exec(writer, "begin transaction;", NULL, NULL, NULL);
}

void db_commit_transaction() {
    if (!db_enabled) {
        return;
    }
    sqlite3_exec(writer, "commit transaction;", NULL, NULL, NULL);
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
    sqlite3_exec(reader, "delete from state;", NULL, NULL, NULL);
    sqlite3_prepare_v2(reader, query, -1, &stmt, NULL);
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
    sqlite3_prepare_v2(reader, query, -1, &stmt, NULL);
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
    mtx_lock(&mtx);
    ring_put(&ring, p, q, x, y, z, w, 0);
    cnd_signal(&cnd);
    mtx_unlock(&mtx);
}

void _db_insert_block(int p, int q, int x, int y, int z, int w) {
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
    sqlite3_reset(load_map_stmt);
    sqlite3_bind_int(load_map_stmt, 1, p);
    sqlite3_bind_int(load_map_stmt, 2, q);
    while (sqlite3_step(load_map_stmt) == SQLITE_ROW) {
        int x = sqlite3_column_int(load_map_stmt, 0);
        int y = sqlite3_column_int(load_map_stmt, 1);
        int z = sqlite3_column_int(load_map_stmt, 2);
        int w = sqlite3_column_int(load_map_stmt, 3);
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
    mtx_lock(&mtx);
    ring_put(&ring, p, q, 0, 0, 0, 0, key);
    cnd_signal(&cnd);
    mtx_unlock(&mtx);
}

void _db_set_key(int p, int q, int key) {
    sqlite3_reset(set_key_stmt);
    sqlite3_bind_int(set_key_stmt, 1, p);
    sqlite3_bind_int(set_key_stmt, 2, q);
    sqlite3_bind_int(set_key_stmt, 3, key);
    sqlite3_step(set_key_stmt);
}
