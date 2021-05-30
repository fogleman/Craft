#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <math.h>

#include "../src/item.h"

#include <CUnit/CUnit.h>
#include "item_test_mutant.h"

static void handleValidTransparentMutant(){
    CU_ASSERT_FALSE(is_transparent(0));
    CU_ASSERT_FALSE(is_transparent(10));
    CU_ASSERT_FALSE(is_transparent(15));

}

static void handleInvalidTransparentMutant(){
    for (int i = 1; i<64; i++){                
        if(i!=10 && i!=15 && i!=17 && i!=18 && i!=19 && i!=20 && i!=21 && i!=22 && i!=23 ) //apparently plants are transparent
            CU_ASSERT(is_transparent(i));     
   
    }

}

static void handleValidObstacleMutant(){
    for (int i = 1; i<64; i++){                
        if( i<16 || i>23 ) //apparently plants are transparent
            CU_ASSERT_FALSE(is_obstacle(i));                        //mutated
    }
}

static void handleInvalidObstacleMutant(){
    for (int i = 16; i<24; i++){
        CU_ASSERT(is_obstacle(i));                                  //mutated
    }
    
}

static void handleValidDestructableMutant(){
    for (int i = 1; i<64; i++){ 
        if(i != 16)
            CU_ASSERT_FALSE(is_destructable(i));                    //mutated
    }

}

static void handleInvalidDestructableMutant(){                            //mutated
    CU_ASSERT(is_destructable(0));    
    CU_ASSERT(is_destructable(16));                                 //mutated

}

static void handleValidPlantMutant(){
    for (int i = 17; i<24; i++){
        CU_ASSERT_FALSE(is_plant(i));                               //mutated
    }

}

static void handleInvalidPlantMutant(){
    for (int i = 0; i<64; i++){
        if(i<17 || i>23)
            CU_ASSERT(is_plant(i));
    }
}


static CU_TestInfo transparency_tests[] = {
    {"Properly handles valid transparent items mutant", handleValidTransparentMutant},
    {"Properly handles invalid transparent items mutant", handleInvalidTransparentMutant},
    CU_TEST_INFO_NULL
};

static CU_TestInfo destructable_tests[] = {
    {"Properly handles valid destructable items mutant", handleValidDestructableMutant},
    {"Properly handles invalid destructable items mutant", handleInvalidDestructableMutant},
    CU_TEST_INFO_NULL
};

static CU_TestInfo obstacle_tests[] = {
    {"Properly handles valid obstacle items mutant", handleValidObstacleMutant},
    {"Properly handles invalid obstacle items mutant", handleInvalidObstacleMutant},
    CU_TEST_INFO_NULL
};

static CU_TestInfo plant_tests[] = {
    {"Properly handles valid plant items mutant", handleValidPlantMutant},
    {"Properly handles invalid plant items mutant", handleInvalidPlantMutant},
    CU_TEST_INFO_NULL

};

static CU_SuiteInfo suites[] = {
    {"transparency suite", NULL, NULL, NULL, NULL, transparency_tests},
    {"obstacle suite", NULL, NULL, NULL, NULL, obstacle_tests},
    {"destructable suite", NULL, NULL, NULL, NULL, destructable_tests},
    {"plant suite", NULL, NULL, NULL, NULL, plant_tests},
    CU_SUITE_INFO_NULL
};

void ItemTestMutant_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}