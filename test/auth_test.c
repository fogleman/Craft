#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include <CUnit/CUnit.h>
#include "auth_test.h"

#include "../src/auth.h"

char* test_user_name 	  = "we-dem-boys";
char* test_identity_token = "e8502b7afe8c408d9621474f33905d9d"; 


static void handles_valid_user() {
	char buffer[2048];

	int result_code = get_access_token(buffer, strlen(test_user_name), test_user_name, test_identity_token);

	CU_ASSERT(result_code);
} 

static void handles_invalid_user() {
	CU_PASS("Implement Later");
}

static void handles_valid_user_with_invalid_identity_token() {
	CU_PASS("Implement Later");
}

static void handles_invalid_user_with_invalid_identity_token() {
	CU_PASS("Implement Later");
}

static CU_TestInfo test_get_auth[] = {
	{"handles_valid_users_authenticating", handles_valid_user},
	{"handles_invalid_users_correctly", handles_invalid_user},
	CU_TEST_INFO_NULL
};

static CU_TestInfo test_unknown_identity_token[] = {
	CU_TEST_INFO_NULL
};


//{suite name, init func, cleanup func, setup func, teardown func}
static CU_SuiteInfo suites[] = {
	{"gets_access_token", NULL, NULL, NULL, NULL, test_get_auth},
	CU_SUITE_INFO_NULL
};


void AuthTest_AddTests() {
	assert(NULL != CU_get_registry());
	assert(!CU_is_test_running());

	if(CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
		exit(EXIT_FAILURE);
	}
}
