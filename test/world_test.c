#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/world.h"

#include <CUnit/CUnit.h>
#include "world_test.h"
#include "../src/map.h"

void map_set_func(int x, int y, int z, int w, void *arg) {
    Map *map = (Map *)arg;
    map_set(map, x, y, z, w);
}

static void makes_consistent_maps() {
    int p = 1, q = 1;
    Map arg1;
    Map arg2;

    map_alloc(&arg1, 10, 10, 10, 1);
    map_alloc(&arg2, 10, 10, 10, 1);


    create_world(p, q, map_set_func, &arg1);
    create_world(p, q, map_set_func, &arg2);

    // map->mask + 1 in length
    MapEntry* mapEntryArg1 = arg1.data;
    MapEntry* mapEntryArg2 = arg2.data;

    int flag = 1;
    for (int i = 0; i < (arg1.mask); i++) {
      if (arg1.data[i].value != arg2.data[i].value) {
        flag = 0;
      }
    }

    CU_ASSERT(flag);
}

static CU_TestInfo world_generation_tests[] = {
    {"checking consistent map creation", makes_consistent_maps},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"world generation suite", NULL, NULL, NULL, NULL, world_generation_tests},
    CU_SUITE_INFO_NULL
};

void WorldTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
