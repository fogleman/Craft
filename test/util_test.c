#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/util.h"

#include <CUnit/CUnit.h>
#include "util_test.h"

static void properly_generates_int() {

    int randArray[5];
    int randArray2[5];
    for (int i = 0; i < 5; i++) {
      randArray[i] = rand_int(100);
      randArray2[i] = rand_int(100);
    }
    CU_ASSERT_FALSE(randArray[0] == randArray2[0] && randArray[1] == randArray2[1] &&
      randArray[2] == randArray2[2] && randArray[3] == randArray2[3] && randArray[5] == randArray2[5]);
}

static void properly_generates_double() {

    double randArray[5];
    double randArray2[5];
    for (int i = 0; i < 5; i++) {
      randArray[i] = rand_double();
      randArray2[i] = rand_double();
    }
    CU_ASSERT_FALSE(randArray[0] == randArray2[0] && randArray[1] == randArray2[1] &&
      randArray[2] == randArray2[2] && randArray[3] == randArray2[3] && randArray[5] == randArray2[5]);
}

static void check_char_width () {

    CU_ASSERT(char_width('a') == 7);
    CU_ASSERT(char_width('~') == 7);
    CU_ASSERT(char_width('0') == 6);
    CU_ASSERT(char_width('G') == 6);
}

static void check_string_width () {

    char charray[5] = {'a', 'b', 'c', 'd', 'e'};
    char charray1[5] = {'1', '2', '3', '4', '5'};
    CU_ASSERT(string_width(&charray) == 31);
    CU_ASSERT(string_width(&charray1) == 27)
}

static void check_tokenize_handles_null () {

    char* str = NULL;
    char* delim = NULL;
    char** key = NULL;
    CU_FAIL("Does not check for null");
    //CU_ASSERT(tokenize(str, delim, key) == NULL);
}

static CU_TestInfo random_generation_tests[] = {
    {"checking random generation of ints", properly_generates_int},
    {"checking random generation of doubles", properly_generates_double},
    CU_TEST_INFO_NULL
};

static CU_TestInfo char_tests[] = {
    {"checking accurate char widths", check_char_width},
    {"checking accurate string widths", check_string_width},
    {"checking null tokenization", check_tokenize_handles_null},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"random generation suite", NULL, NULL, NULL, NULL, random_generation_tests},
    {"char test suite", NULL, NULL, NULL, NULL, char_tests},
    CU_SUITE_INFO_NULL
};

void UtilTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
