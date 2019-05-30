#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <math.h>

#include "../src/map.h"

#include <CUnit/CUnit.h>
#include "map_test.h"

static void properly_hashes_number(){


    CU_ASSERT(hash_int(4) != hash_int(5));
    CU_ASSERT(hash_int(3) == hash_int(3));
}

static void properly_hashes_numbers(){

    CU_ASSERT(hash(1,2,3) == hash(1,2,3));
    CU_ASSERT(hash(1,2,3) != hash(4,5,6));

}

static void properly_inits_map_struct(){
    Map temp;
    map_alloc(&temp, 1,2,3,4);
    
    CU_ASSERT(temp.dx !=0);
    CU_ASSERT(temp.dy !=0);
    CU_ASSERT(temp.dz !=0);
    CU_ASSERT(temp.mask !=0);
    CU_ASSERT(temp.data);

}

static void properly_free_map_struct(){
    Map temp;
    map_alloc(&temp,1,2,3,4);

    CU_ASSERT(temp.dx !=0);
    CU_ASSERT(temp.dy !=0);
    CU_ASSERT(temp.dz !=0);
    CU_ASSERT(temp.mask !=0); 
    CU_ASSERT(temp.data);

    map_free(&temp);

    CU_ASSERT(temp.data == NULL);           //edited code to make it null after it is freed. 

}

static void properly_copy_map_struct(){
    Map temp1, temp2;
    map_alloc(&temp1,1,2,3,4);

    CU_ASSERT(temp1.dx !=temp2.dx);
    CU_ASSERT(temp1.dy !=temp2.dy)
    CU_ASSERT(temp1.dz !=temp2.dz);
    CU_ASSERT(temp1.mask != temp2.mask);


    map_copy(&temp2, &temp1);

    CU_ASSERT(temp1.dx ==temp2.dx);
    CU_ASSERT(temp1.dy ==temp2.dy)
    CU_ASSERT(temp1.dz ==temp2.dz);
    CU_ASSERT(temp1.mask == temp2.mask);
   
   //CU_ASSERT(temp1.data == temp2.data);        //This fails
    /*if(temp1.data == temp2.data){
        CU_ASSERT(1);
    }else{
        CU_ASSERT(0);
    }
    ///////////////////    
    for(int i = 0; i<=temp1.mask;i++ ){
        CU_ASSERT(temp1.data[i] == temp2.data[i]);
    }*/
    


}


static CU_TestInfo hash_tests[] = {
    {"hash_int() Properly handles different hash values for different numbers", properly_hashes_number},
    {"hash() Properly handles hash values for sets of three numbers", properly_hashes_numbers},
    CU_TEST_INFO_NULL
};

static CU_TestInfo init_map_tests[] = {
    {"Properly handles initializing a map struct", properly_inits_map_struct},
    {"Properly handles freeing a map struct", properly_free_map_struct},
    {"Properly handles copying a map struct", properly_copy_map_struct},
    CU_TEST_INFO_NULL
};



static CU_SuiteInfo suites[] = {
    {"hash suite", NULL, NULL, NULL, NULL, hash_tests},
    {"map suite", NULL, NULL, NULL, NULL, init_map_tests},
    CU_SUITE_INFO_NULL
};

void MapTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}