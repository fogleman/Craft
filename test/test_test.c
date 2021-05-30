#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <CUnit/CUnit.h>
#include "test_test.h"

static int suite_success_init() { return 0; }
static int suite_success_clean() { return 0; }

static int suite_failure_init() { return 1; }
static int suite_failure_clean() { return 1; }

static void test_success_one() {
	CU_ASSERT(1);
}

static void test_failure_one() {
	CU_ASSERT(0);
}

static CU_TestInfo tests_success[] = {
	{"test_success_one", test_success_one},
	CU_TEST_INFO_NULL
};

static CU_TestInfo tests_failure[] = {
	{"test_failure_one", test_failure_one},
	CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
	{"suite_success_both", suite_success_init, suite_success_clean, NULL, NULL, tests_success},
	{"suite_success_init", suite_success_init, NULL, NULL, NULL, tests_success},
	{"suite_success_clean", NULL, suite_success_clean, NULL, NULL, tests_success},
	CU_SUITE_INFO_NULL
}; 

void TestTest_AddTests() {
	assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());

	if(CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
		exit(EXIT_FAILURE);
	}	
}

void TestTest_example_results() {
	// pass
}

