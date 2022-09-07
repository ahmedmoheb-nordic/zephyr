/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include "testing_common_defs.h"
#include <keys.h>
#include "host_mocks/assert.h"
#include "mocks/rpa.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID, Address and key type.
 * Item in this list will be used to fill keys pool.
 */
static const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED] = {

	{ BT_ADDR_ID_1, BT_ADDR_LE_1},
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_1},
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_2},
	{ BT_ADDR_ID_1, BT_ADDR_LE_3},

	{ BT_ADDR_ID_2, BT_ADDR_LE_1},
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_2},
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_3},
	{ BT_ADDR_ID_2, BT_ADDR_LE_2},

	{ BT_ADDR_ID_3, BT_ADDR_LE_1},
	{ BT_ADDR_ID_3, BT_ADDR_LE_2},

	{ BT_ADDR_ID_4, BT_ADDR_LE_1}
};

/* Global iterator to iterate over the ID, Address pairs LUT */
static uint8_t params_it;

/* Pointer to the current set of testing parameters */
struct id_addr_pair const *current_params_vector;

/* This list will hold returned references while filling keys pool */
static struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Find a non-existing key reference for ID and Address pair
 *
 *  Constraints:
 *   - Empty keys pool list
 *   - ID and address pair doesn't exist in the keys pool
 *
 *  Expected behaviour:
 *   - A NULL value is returned
 */
void test_find_non_existing_key_reference(void)
{
	struct bt_keys *returned_ref;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_ref = bt_keys_find_addr(id, addr);

	zassert_true(returned_ref == NULL, "bt_keys_find_addr() returned a non-valid reference");
}

/*
 *  Find an existing key reference by ID and Address
 *
 *  Constraints:
 *   - Full keys pool list
 *   - ID and device address match
 *
 *  Expected behaviour:
 *   - A valid reference value is returned
 */
void test_find_existing_key_reference_by_id_and_address(void)
{
	struct bt_keys *returned_ref, *expected_key_ref;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_ref = bt_keys_find_addr(id, addr);
	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	zassert_true(returned_ref != NULL, "bt_keys_find_addr() returned a NULL reference");
	zassert_equal_ptr(returned_ref, expected_key_ref,
			  "bt_keys_find_addr() returned unexpected reference");
}

/* Initialize test variables */
static void reset_global_test_round_params(void)
{
	params_it = 0;
	current_params_vector = NULL;
}

/* Fill the keys pool with testing (ID, Address) pairs */
static void keys_fill_pool(void)
{
	uint8_t id;
	bt_addr_le_t *addr;
	struct id_addr_pair const *params_vector;

	zassert_true(check_key_pool_is_empty(),
		     "List isn't empty, make sure to run this test just after a fresh start");

	for (size_t it = 0; it < ARRAY_SIZE(testing_id_addr_pair_lut); it++) {
		params_vector = &testing_id_addr_pair_lut[it];
		id = params_vector->id;
		addr = params_vector->addr;
		returned_keys_refs[it] = bt_keys_get_addr(id, addr);
		zassert_true(returned_keys_refs[it] != NULL,
			     "bt_keys_get_addr() returned a non-valid reference");
	}
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

	ztest_test_suite(bt_keys_find_addr_non_existing_key,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_non_existing_key_reference));

	ztest_run_test_suite(bt_keys_find_addr_non_existing_key);

	reset_global_test_round_params();
	clear_key_pool();
	keys_fill_pool();

	ztest_test_suite(bt_keys_find_addr_existing_key_matched_id_address,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_existing_key_reference_by_id_and_address));

	ztest_run_test_suite(bt_keys_find_addr_existing_key_matched_id_address);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
