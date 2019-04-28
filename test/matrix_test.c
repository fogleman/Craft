#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "../src/matrix.h"

#include <CUnit/CUnit.h>
#include "matrix_test.h"

#define EPSILON 0.0001

static int float_equals(float a, float b) { return abs(a - b) < EPSILON; }

static void properly_normalizes_non_zero_vector3() {

    float ox, oy, oz;

    ox = 4; 
    oy = 5;
    oz = 7;

    normalize(&ox, &oy, &oz);
    
    float sqr_length = ox*ox + oy*oy + oz*oz;

    CU_ASSERT(float_equals(sqr_length, 1));
}

static void properly_handles_zero_vector3() {

    float ox, oy, oz;

    ox = oy = oz = 0;

    normalize(&ox, &oy, &oz);

    float sqr_length = ox*ox + oy*oy + oz*oz;
   
    CU_TEST_FATAL(sqr_length);
}

static void properly_constructs_identity_matrix() {
    float matrix[16];
    mat_identity(matrix);

    // four component vector
    float vector[4];

    vector[0] = 3;
    vector[1] = 4;
    vector[2] = 5;
    vector[3] = 89;

    // three component vector
    float result[4];

    mat_vec_multiply(result, matrix, vector);

    CU_ASSERT(float_equals(vector[0], result[0]));
    CU_ASSERT(float_equals(vector[1], result[1]));
    CU_ASSERT(float_equals(vector[2], result[2]));
    CU_ASSERT(float_equals(vector[3], result[3]));
}

static void properly_constructs_translation_matrix() {
    float matrix[16];
    mat_translate(matrix, -4, 5, 90);

    float vector[4];
    vector[0] = 5;
    vector[1] = 6;
    vector[2] = 7;
    // note for translating a point in R3, the last component needs to be 
    // a one. translations aren't really linear transforms, so translating 
    // with a matrix is somewhat of a hack
    vector[3] = 1;

    float result[4];

    mat_vec_multiply(result, matrix, vector);

    CU_ASSERT(float_equals(result[0], 1));
    CU_ASSERT(float_equals(result[1], 11));
    CU_ASSERT(float_equals(result[2], 97));
    CU_ASSERT(float_equals(result[3], vector[3]));
}



static CU_TestInfo normalization_tests[] = {
    {"normalization works for non zero vectors", properly_normalizes_non_zero_vector3},
    {"normalization handles zero vector", properly_handles_zero_vector3},
    CU_TEST_INFO_NULL
};

static CU_TestInfo matrix_product_tests[] = {
    {"properly constructs the identity matrix", properly_constructs_identity_matrix},
    {"properly constructs translation matricies", properly_constructs_translation_matrix},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"normalization suite", NULL, NULL, NULL, NULL, normalization_tests},
    {"matrix product suite", NULL, NULL, NULL, NULL, matrix_product_tests},
    CU_SUITE_INFO_NULL
};

void MatrixTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
