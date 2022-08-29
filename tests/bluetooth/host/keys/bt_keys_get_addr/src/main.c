/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "host_mocks/print_utils.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID and Address pairs */
const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED] = {
	{ BT_ADDR_ID_1, BT_ADDR_LE_1 },
	{ BT_ADDR_ID_1, BT_ADDR_LE_2 },
	{ BT_ADDR_ID_2, BT_ADDR_LE_1 },
	{ BT_ADDR_ID_2, BT_ADDR_LE_2 }
};

/* Global iterator to iterate over the ID, Address pairs LUT */
static uint8_t params_it;

/* Pointer to the current set of testing parameters */
struct id_addr_pair const *current_params_vector;

/* This list will hold returned references while filling keys pool */
struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Test list is empty
 *
 *  Constraints:
 *   - Empty list at start up
 *
 *  Expected behaviour:
 *   - List is empty at start up
 */
void test_list_is_empty_at_startup(void)
{
	zassert_true(check_key_pool_is_empty(),
		     "List isn't empty, make sure to run this test just after a fresh start");
}

/*
 *  Test filling the keys pool with (ID, Address) pairs
 *
 *  Constraints:
 *   - Empty list at startup
 *
 *  Expected behaviour:
 *   - A valid reference is returned by bt_keys_get_addr()
 *   - ID value matches the one passed to bt_keys_get_addr()
 *   - Address value matches the one passed to bt_keys_get_addr()
 */
void test_get_non_existing_keys(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = current_params_vector->id;

	bt_addr_le_copy(&addr, current_params_vector->addr);

	returned_key = bt_keys_get_addr(id, &addr);
	/* 'params_it' is incremented before running the test case */
	returned_keys_refs[params_it - 1] = returned_key;

	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");
	zassert_true(returned_key->id == id,
		     "bt_keys_get_addr() returned a reference with an incorrect ID");
	zassert_true(!bt_addr_le_cmp(&returned_key->addr, &addr),
		     "bt_keys_get_addr() set incorrect address %s value, expected %s",
		     bt_addr_le_str(&returned_key->addr), bt_addr_le_str(&addr));
	zassert_false(check_key_pool_is_empty(),
		      "List is empty, (ID, Address) pair insertion failed");
}

/*
 *  Test no equal references returned by bt_keys_get_addr()
 *
 *  Constraints:
 *   - Keys pool has been filled
 *
 *  Expected behaviour:
 *   - All returned references are different from each other
 */
void test_no_equal_references(void)
{
	struct bt_keys *keys_pool = bt_keys_get_key_pool();

	for (uint32_t i = 0; i < ARRAY_SIZE(returned_keys_refs); i++) {
		struct bt_keys *returned_ref = returned_keys_refs[i];

		zassert_equal_ptr(keys_pool + i, returned_ref,
				  "bt_keys_get_addr() returned unexpected reference at slot %u", i);
	}
}

/*
 *  Test getting a valid key reference by a matching ID and address
 *
 *  Constraints:
 *   - ID and address pair has been inserted in the list
 *
 *  Expected behaviour:
 *   - A valid reference is returned by bt_keys_get_addr() that
 *     matches the one returned after adding the ID and address pair
 *   - ID value matches the one passed to bt_keys_get_addr()
 *   - Address value matches the one passed to bt_keys_get_addr()
 */
void test_get_key_by_matched_id_and_address(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key, *expected_key_ref;
	uint8_t id = current_params_vector->id;

	bt_addr_le_copy(&addr, current_params_vector->addr);

	returned_key = bt_keys_get_addr(id, &addr);
	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");
	zassert_equal_ptr(returned_key, expected_key_ref,
			  "bt_keys_get_addr() returned unexpected reference");
}

/* Initialize test variables */
static void reset_global_test_round_params(void)
{
	params_it = 0;
	current_params_vector = NULL;
}

/* Setup test variables */
static void unit_test_setup(void)
{
	zassert_true((params_it < (ARRAY_SIZE(testing_id_addr_pair_lut))),
		     "Invalid testing parameters index %u", params_it);
	current_params_vector = &testing_id_addr_pair_lut[params_it];
	params_it++;
}

void test_main(void)
{
	reset_global_test_round_params();

	/* This testsuite should setup the conditions for subsequent tests
	 *   - Checks the initial startup state of the keys pool to be empty
	 *   - Test bt_keys_get_addr() behaviour when trying to get a key
	 *     reference for non-existing (ID, Address) pair
	 */
	ztest_test_suite(bt_keys_get_addr_get_non_existing_keys,
			 ztest_unit_test(test_list_is_empty_at_startup),
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_get_non_existing_keys),
			 ztest_unit_test(test_no_equal_references));

	ztest_run_test_suite(bt_keys_get_addr_get_non_existing_keys);

	reset_global_test_round_params();

	/* Checks when (ID, Address) pairs exist in keys pool */
	ztest_test_suite(bt_keys_get_addr_get_existing_keys,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_get_key_by_matched_id_and_address));

	ztest_run_test_suite(bt_keys_get_addr_get_existing_keys);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
