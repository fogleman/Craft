#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "test_test.h"
#include "ring_test.h"
#include "ring_test_mutate.h"
#include "auth_test.h"
#include "world_test.h"
#include "util_test.h"
#include "matrix_test.h"


void AddAllTests() {
	TestTest_AddTests();
	RingTest_AddTests();
	RingTestMutate_AddTests();
	AuthTest_AddTests();
	WorldTest_AddTests();
	UtilTest_AddTests();
  MatrixTest_AddTests();
}

int main(int argc, char** argv) {



	if(CU_initialize_registry()) {
		printf("\nInitialization of test registry failed.\n");
	} else {
		AddAllTests();
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_set_error_action(CUEA_IGNORE);
		printf("\ntests completed with return value %d. \n", CU_basic_run_tests());
		CU_cleanup_registry();
	}

	return 0;
}
