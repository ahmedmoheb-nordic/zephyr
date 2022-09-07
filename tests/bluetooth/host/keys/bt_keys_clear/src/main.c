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
#include "mocks/id.h"
#include "mocks/id_expects.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID, Address and key type.
 * Item in this list will be used to fill keys pool.
 */
static const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED] = {

	{ BT_ADDR_ID_1, BT_ADDR_LE_1 },
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_1 },
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_2 },
	{ BT_ADDR_ID_1, BT_ADDR_LE_3 },

	{ BT_ADDR_ID_2, BT_ADDR_LE_1 },
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_2 },
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_3 },
	{ BT_ADDR_ID_2, BT_ADDR_LE_2 },

	{ BT_ADDR_ID_3, BT_ADDR_LE_1 },
	{ BT_ADDR_ID_3, BT_ADDR_LE_2 },

	{ BT_ADDR_ID_4, BT_ADDR_LE_1 }
};

/* Global iterator to iterate over the ID, Address and type LUT */
static uint8_t params_it;

/* Pointer to the current set of testing parameters */
struct id_addr_pair const *current_params_vector;

/* This list will hold returned references while filling keys pool */
static struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Clear an existing key and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *
 *  Expected behaviour:
 *   - The key content is cleared
 */
void test_verify_key_cleared_correctly(void)
{
	struct bt_keys empty_key;
	struct bt_keys *key_ref_to_clear, *find_returned_ref;
	uint8_t id_to_find = current_params_vector->id;
	const bt_addr_le_t *addr_to_find = current_params_vector->addr;

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		ztest_test_skip();
	}

	memset(&empty_key, 0x00, sizeof(struct bt_keys));
	/* 'params_it' is incremented before running the test case */
	key_ref_to_clear = returned_keys_refs[params_it - 1];

	/* Ensure that item exists in the keys pool */
	find_returned_ref = bt_keys_find_addr(id_to_find, addr_to_find);
	zassert_true(find_returned_ref != NULL, "bt_keys_find_addr() returned a NULL reference");

	bt_keys_clear(key_ref_to_clear);

	expect_not_called_bt_id_del();

	/* Verify that memory was cleared */
	zassert_mem_equal(key_ref_to_clear, &empty_key, sizeof(struct bt_keys),
			  "Key content wasn't cleared by 'bt_keys_clear()'");

	/* Ensure that item doesn't exist in the keys pool after calling bt_keys_clear() */
	find_returned_ref = bt_keys_find_addr(id_to_find, addr_to_find);
	zassert_true(find_returned_ref == NULL,
		     "bt_keys_find_addr() returned a non-NULL reference");
}

/*
 *  Clear an existing key and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *
 *  Expected behaviour:
 *   - The key content is cleared
 *   - bt_id_del() is called with correct key reference
 */
void test_verify_key_cleared_correctly_and_bt_id_del_called(void)
{
	struct bt_keys empty_key;
	struct bt_keys *key_ref_to_clear, *find_returned_ref;
	uint8_t id_to_find = current_params_vector->id;
	const bt_addr_le_t *addr_to_find = current_params_vector->addr;

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		ztest_test_skip();
	}

	memset(&empty_key, 0x00, sizeof(struct bt_keys));
	/* 'params_it' is incremented before running the test case */
	key_ref_to_clear = returned_keys_refs[params_it - 1];

	/* Ensure that item exists in the keys pool */
	find_returned_ref = bt_keys_find_addr(id_to_find, addr_to_find);
	zassert_true(find_returned_ref != NULL, "bt_keys_find_addr() returned a NULL reference");

	bt_keys_clear(key_ref_to_clear);

	expect_single_call_bt_id_del(key_ref_to_clear);

	/* Verify that memory was cleared */
	zassert_mem_equal(key_ref_to_clear, &empty_key, sizeof(struct bt_keys),
			  "Key content wasn't cleared by 'bt_keys_clear()'");

	/* Ensure that item doesn't exist in the keys pool after calling bt_keys_clear() */
	find_returned_ref = bt_keys_find_addr(id_to_find, addr_to_find);
	zassert_true(find_returned_ref == NULL,
		     "bt_keys_find_addr() returned a non-NULL reference");
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

/* Set keys state to BT_KEYS_ID_ADDED */
static void keys_set_state(void)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(returned_keys_refs); i++) {
		returned_keys_refs[i]->state = BT_KEYS_ID_ADDED;
	}
}

/* Setup test variables */
static void unit_test_setup(void)
{
	zassert_true((params_it < (ARRAY_SIZE(testing_id_addr_pair_lut))),
		     "Invalid testing parameters index %u", params_it);
	current_params_vector = &testing_id_addr_pair_lut[params_it];
	params_it++;

	/* Register resets */
	ID_FFF_FAKES_LIST(RESET_FAKE);
}

void test_main(void)
{

	reset_global_test_round_params();
	clear_key_pool();
	keys_fill_pool();

	ztest_test_suite(bt_keys_clear_existing_keys,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_verify_key_cleared_correctly));

	ztest_run_test_suite(bt_keys_clear_existing_keys);

	reset_global_test_round_params();
	clear_key_pool();
	keys_fill_pool();
	keys_set_state();

	ztest_test_suite(bt_keys_clear_existing_keys_with_state,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_verify_key_cleared_correctly_and_bt_id_del_called));

	ztest_run_test_suite(bt_keys_clear_existing_keys_with_state);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
