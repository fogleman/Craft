#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "../src/sign.h"

#include <CUnit/CUnit.h>
#include "sign_test.h"

#define EPSILON 0.0001
#define M_PI 3.14159265358979323846

static void alloc_properly_allocates_memory() {
	SignList list;
	int capacity = 5;
	
	sign_list_alloc(&list, capacity);
	
	CU_ASSERT(list.capacity == 5);
	CU_ASSERT(list.data != NULL);
	CU_ASSERT(list.size == 0);
}

static void can_grow_sign_list() {
	SignList list;
	
	sign_list_alloc(&list, 5);
	sign_list_grow(&list);
	
	CU_ASSERT(list.capacity == 10);
	CU_ASSERT(list.data != NULL);
	CU_ASSERT(list.size == 0);
}

static void can_add_sign_to_sign_list() {
	SignList list;
	
	sign_list_alloc(&list, 1);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	
	CU_ASSERT(list.size == 1);
	CU_ASSERT(list.capacity == 1);
	CU_ASSERT(list.data[0].x == 1);
	CU_ASSERT(list.data[0].y == 2);
	CU_ASSERT(list.data[0].z == 3);
	CU_ASSERT(list.data[0].face == 4);
	CU_ASSERT(strcmp(list.data[0].text, "test") == 0);
}

static void sign_list_grows_when_needed() {
	SignList list;
	
	sign_list_alloc(&list, 1);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 2, 3, 4, 5, "test2");
	
	CU_ASSERT(list.size == 2);
	CU_ASSERT(list.capacity == 2);
}

static void sign_list_ignores_duplicates() {
	SignList list;
	
	sign_list_alloc(&list, 1);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 1, 2, 3, 4, "test");
	
	CU_ASSERT(list.size == 1);
	CU_ASSERT(list.capacity == 1);
}

static void can_remove_a_sign() {
	SignList list;
	
	sign_list_alloc(&list, 1);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	
	sign_list_remove(&list, 1, 2, 3, 4);
	
	CU_ASSERT(list.size == 0);
}

static void can_remove_specific_sign() {
	SignList list;
	
	sign_list_alloc(&list, 2);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 2, 3, 4, 5, "test2");

	sign_list_remove(&list, 1, 2, 3, 4);
	
	CU_ASSERT(list.size == 1);
}

static void can_remove_all_signs() {
	SignList list;
	
	sign_list_alloc(&list, 4);
	sign_list_add(&list, 1, 2, 3, 4, "test");
	sign_list_add(&list, 1, 2, 3, 5, "test2");
	sign_list_add(&list, 3, 4, 5, 6, "test3");
	sign_list_add(&list, 4, 5, 6, 7, "test4");
	
	CU_ASSERT(sign_list_remove_all(&list, 1, 2, 3) == 2);
	CU_ASSERT(list.size == 2);
	
}

static CU_TestInfo sign_tests[] = {
    {"memory allocation of new SignList works properly", alloc_properly_allocates_memory},
	{"can grow an already existing SignList", can_grow_sign_list},	
	{"can add a new sign to a SignList", can_add_sign_to_sign_list},
	{"SignList grows when too many signs are added", sign_list_grows_when_needed},
	{"SignList does not add duplicate signs", sign_list_ignores_duplicates},
	{"can remove a sign from a SignList", can_remove_a_sign},
	{"can remove a specific sign from a SignList", can_remove_specific_sign},
	{"can mass remove signs", can_remove_all_signs},
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    {"sign suite", NULL, NULL, NULL, NULL, sign_tests},
    CU_SUITE_INFO_NULL
};

void SignTest_AddTests() {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if(CU_register_suites(suites) != CUE_SUCCESS) {
        fprintf(stderr, "suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}