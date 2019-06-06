#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/ring.h"

#include <CUnit/CUnit.h>
#include "ring_test.h"

static void properly_checks_ring_empty() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    CU_ASSERT(ring_empty(&ring))

    ring.end = 7;
    CU_ASSERT_FALSE(ring_empty(&ring));

    ring_free(&ring);
}

static void properly_checks_ring_full() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    ring.end = 7;
    CU_ASSERT(ring_full(&ring))

    ring.end = 6;
    CU_ASSERT_FALSE(ring_full(&ring));

    ring_free(&ring);
}

static void properly_checks_ring_size() {

    Ring ring;
    int cap = 8;
    int en = 4;
    ring_alloc(&ring, cap);

    ring.end = en;
    CU_ASSERT(ring_size(&ring) == 4);

    ring_free(&ring);
}

static void properly_handles_zero_length() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    CU_ASSERT(ring_size(&ring) == 0);
    CU_ASSERT_FALSE(ring_full(&ring));

    ring_free(&ring);
}

static void properly_gets_ring() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    RingEntry ent;

    CU_ASSERT_FALSE(ring_get(&ring, &ent));

    ring.end = 7;

    CU_ASSERT(ring_get(&ring, &ent));

    ring_free(&ring);

}

static void properly_grows_ring() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    ring_grow(&ring);

    CU_ASSERT(ring.capacity > 8);
}

static void properly_puts_block() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    int p, q, x, y, z, w;
    p = q = x = y = z = w = 1;

    ring_put_block(&ring, p, q, x, y, z, w);

    CU_ASSERT(ring.data->type == BLOCK);
    CU_ASSERT(ring.data->p);
    CU_ASSERT(ring.data->q);
    CU_ASSERT(ring.data->x);
    CU_ASSERT(ring.data->y);
    CU_ASSERT(ring.data->z);
    CU_ASSERT(ring.data->w);

    ring_free(&ring);
}

static void properly_puts_light() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    int p, q, x, y, z, w;
    p = q = x = y = z = w = 1;

    ring_put_light(&ring, p, q, x, y, z, w);

    CU_ASSERT(ring.data->type == LIGHT);
    CU_ASSERT(ring.data->p);
    CU_ASSERT(ring.data->q);
    CU_ASSERT(ring.data->x);
    CU_ASSERT(ring.data->y);
    CU_ASSERT(ring.data->z);
    CU_ASSERT(ring.data->w);

    ring_free(&ring);
}

static void properly_puts_key() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    int p, q, key;
    p = q = key = 1;

    ring_put_key(&ring, p, q, key);

    CU_ASSERT(ring.data->type == KEY);
    CU_ASSERT(ring.data->p);
    CU_ASSERT(ring.data->q);
    CU_ASSERT(ring.data->key);

    ring_free(&ring);
}

static void properly_puts_commit() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    ring_put_commit(&ring);

    CU_ASSERT(ring.data->type == COMMIT);

    ring_free(&ring);
}

static void properly_puts_exit() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    ring_put_exit(&ring);

    CU_ASSERT(ring.data->type == EXIT);

    ring_free(&ring);
}

static CU_TestInfo ring_analysis_tests[] = {
    {"checking empty ring works", properly_checks_ring_empty},
    {"checking full ring works", properly_checks_ring_full},
    {"checking ring size works", properly_checks_ring_size},
    {"checking zero length ring", properly_handles_zero_length},
    CU_TEST_INFO_NULL
};


static CU_SuiteInfo suites[] = {
    {"ring analysis suite", NULL, NULL, NULL, NULL, ring_analysis_tests},

static CU_TestInfo ring_put_tests[] = {
    {"checking block put", properly_puts_block},
    {"checking light put", properly_puts_light},
    {"checking key put", properly_puts_key},
    {"checking commit put", properly_puts_commit},
    {"checking exit put", properly_puts_exit},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"ring analysis suite", NULL, NULL, NULL, NULL, ring_analysis_tests},
    {"ring put suite", NULL, NULL, NULL, NULL, ring_put_tests},
    CU_SUITE_INFO_NULL
};

void RingTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
