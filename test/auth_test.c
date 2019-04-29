#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include <CUnit/CUnit.h>
#include "auth_test.h"

#include "../src/auth.h"

char* test_user_name 	  = "we-dem-boys";
char* test_identity_token = "e8502b7afe8c408d9621474f33905d9d"; 

char* fake_user_name = "this name is fake";
char* fake_identity_token = "this id token is fake";

static void handles_valid_user() {
	char buffer[2048];

	int result_code = get_access_token(buffer, strlen(test_user_name), test_user_name, test_identity_token);

	CU_ASSERT(result_code);
} 

static void handles_invalid_user() {
	char buffer[2048];

    int result_code = get_access_token(buffer, strlen(fake_user_name), fake_user_name, fake_identity_token);

    CU_ASSERT_FALSE(result_code);
}

static void handles_valid_user_with_invalid_identity_token() {
	char buffer[2048];

    int result_code = get_access_token(buffer, strlen(test_user_name), test_user_name, fake_identity_token);

    CU_ASSERT_FALSE(result_code);
}

static void handles_invalid_user_with_valid_identity_token() {
	char buffer[2048];

    int result_code = get_access_token(buffer, strlen(fake_user_name), fake_user_name, test_identity_token);

    CU_ASSERT_FALSE(result_code);
}

static CU_TestInfo test_get_auth[] = {
	{"handles valid users authenticating", handles_valid_user},
	{"handles invalid users correctly", handles_invalid_user},
    {"handles a valid user without a correct ID token", handles_valid_user_with_invalid_identity_token},
    {"handles an invalid user with a correct ID token", handles_invalid_user_with_valid_identity_token},
	CU_TEST_INFO_NULL
};



//{suite name, init func, cleanup func, setup func, teardown func}
static CU_SuiteInfo suites[] = {
	{"handles retreviing access token", NULL, NULL, NULL, NULL, test_get_auth},
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
