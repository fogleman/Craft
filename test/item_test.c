#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <math.h>

#include "../src/item.h"

#include <CUnit/CUnit.h>
#include "item_test.h"

static void handleValidTransparent(){
    

}

static void handleInvalidTransparent(){


}

static void handleValidObstacle(){


}

static void handleInvalidObstacle(){

    
}

static void handleValidDestructable(){


}

static void handleInvalidDestructable(){


}

static void handleValidPlant(){


}

static void handleInvalidPlant(){



}


static CU_TestInfo transparency_tests[] = {
    {"Properly handles valid transparent items", handleValidTransparent},
    {"Properly handles invalid transparent items", handleInvalidTransparent},
    CU_TEST_INFO_NULL
}

static CU_TestInfo destructable_tests[] = {
    {"Properly handles valid destructable items", handleValidDestructable},
    {"Properly handles invalid destructable items", handleInvalidDestructable},
    CU_TEST_INFO_NULL
}

static CU_TestInfo obstacle_tests[] = {
    {"Properly handles valid obstacle items", handleValidObstacle},
    {"Properly handles invalid obstacle items", handleInvalidObstacle},
    CU_TEST_INFO_NULL
}

static CU_TestInfo plant_tests[] = {
    {"Properly handles valid plant items", handleValidPlant},
    {"Properly handles invalid plant items", handleInvalidPlant},
    CU_TEST_INFO_NULL

}

static CU_SuiteInfo suites[] = {
    {"transparency suite", NULL, NULL, NULL, NULL, transparency_tests},
    {"obstacle suite", NULL, NULL, NULL, NULL, obstacle_tests},
    {"destructable suite", NULL, NULL, NULL, NULL, destructable_tests},
    {"plant suite", NULL, NULL, NULL, NULL, plan_tests},
    CU_SUITE_INFO_NULL
};

void ItemTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}