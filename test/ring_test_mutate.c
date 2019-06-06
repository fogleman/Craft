#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/ring.h"

#include <CUnit/CUnit.h>
#include "ring_test_mutate.h"

static void properly_checks_ring_empty() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    CU_ASSERT_FALSE(ring_empty(&ring))

    ring.end = 7;
    CU_ASSERT(ring_empty(&ring));

    ring_free(&ring);
}

static void properly_checks_ring_full() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    ring.end = 7;
    CU_ASSERT_FALSE(ring_full(&ring))

    ring.end = 6;
    CU_ASSERT(ring_full(&ring));

    ring_free(&ring);
}

static void properly_checks_ring_size() {

    Ring ring;
    int cap = 8;
    int en = 4;
    ring_alloc(&ring, cap);

    ring.end = en;
    CU_ASSERT_FALSE(ring_size(&ring) == 4);

    ring_free(&ring);
}

static void properly_handles_zero_length() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    CU_ASSERT_FALSE(ring_size(&ring) == 0);
    CU_ASSERT(ring_full(&ring));

    ring_free(&ring);
}

static void properly_gets_ring() {

    Ring ring;
    int cap = 8;
    ring_alloc(&ring, cap);

    RingEntry ent;

    CU_ASSERT(ring_get(&ring, &ent));

    ring.end = 7;

    CU_ASSERT_FALSE(ring_get(&ring, &ent));

    ring_free(&ring);

}

static CU_TestInfo ring_analysis_mutate_tests[] = {
    {"checking empty ring works (mutant)", properly_checks_ring_empty},
    {"checking full ring works (mutant)", properly_checks_ring_full},
    {"checking ring size works (mutant)", properly_checks_ring_size},
    {"checking zero length ring (mutant)", properly_handles_zero_length},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"ring analysis suite (mutant)", NULL, NULL, NULL, NULL, ring_analysis_mutate_tests},
    CU_SUITE_INFO_NULL
};

void RingTestMutate_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
