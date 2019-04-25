#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CUnit/Basic.h"
#include "test_test.h"


int main(int argc, char** argv) {



	if(CU_initialize_registry()) {
		printf("\nInitialization of test registry failed.\n");
	} else {
		TestTest_AddTests();
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_set_error_action(CUEA_IGNORE);
		printf("\ntests completed with return value %d. \n", CU_basic_run_tests());
		CU_cleanup_registry();
	}

	return 0;
}
