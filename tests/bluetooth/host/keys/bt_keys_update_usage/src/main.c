/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "mocks/settings_store_expects.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID, Address and key type.
 * Item in this list will be used to fill keys pool.
 */
const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED] = {

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

/* This list will hold returned references while filling keys pool */
struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

/* Holds the last key reference updated */
static struct bt_keys *last_keys_updated;

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Request updating non-existing item
 *
 *  Constraints:
 *   - Keys pool is filled with items
 *   - ID and address pair doesn't exist in the keys pool
 *
 *  Expected behaviour:
 *   - Last updated key reference isn't changed
 */
void test_update_non_existing_key(void)
{
	uint8_t id = BT_ADDR_ID_5;
	bt_addr_le_t *addr = BT_ADDR_LE_5;

	bt_keys_update_usage(id, addr);

	zassert_equal_ptr(bt_keys_get_last_keys_updated(), last_keys_updated,
			  "bt_keys_update_usage() changed last updated key reference unexpectedly");

}

/*
 *  Request updating the latest key reference
 *
 *  Constraints:
 *   - Keys pool is filled with items
 *
 *  Expected behaviour:
 *   - Last updated key reference isn't changed
 */
void test_update_latest_reference(void)
{
	uint8_t id = testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED - 1].id;
	bt_addr_le_t *addr = testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED - 1].addr;

	bt_keys_update_usage(id, addr);

	zassert_equal_ptr(bt_keys_get_last_keys_updated(), last_keys_updated,
			  "bt_keys_update_usage() changed last updated key reference unexpectedly");
}

/*
 *  Request updating the latest key reference
 *
 *  Constraints:
 *   - Keys pool is filled with items
 *   - CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING is enabled
 *
 *  Expected behaviour:
 *   - Last updated key reference matches the last updated key reference
 */
void test_update_non_latest_reference(void)
{
	uint8_t id;
	uint32_t old_aging_counter;
	bt_addr_le_t *addr;
	struct bt_keys *expected_updated_keys;
	struct id_addr_pair const *params_vector;

	if (IS_ENABLED(CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING)) {
		ztest_test_skip();
	}

	for (size_t it = 0; it < ARRAY_SIZE(testing_id_addr_pair_lut); it++) {
		params_vector = &testing_id_addr_pair_lut[it];
		id = params_vector->id;
		addr = params_vector->addr;
		expected_updated_keys = returned_keys_refs[it];
		old_aging_counter = expected_updated_keys->aging_counter;

		bt_keys_update_usage(id, addr);

		zassert_true(expected_updated_keys->aging_counter > (old_aging_counter),
			     "bt_keys_update_usage() set incorrect aging counter");

		zassert_equal_ptr(bt_keys_get_last_keys_updated(), expected_updated_keys,
			"bt_keys_update_usage() changed last updated key reference unexpectedly");

		expect_not_called_settings_save_one();
	}
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
		last_keys_updated = returned_keys_refs[it];
		zassert_true(returned_keys_refs[it] != NULL,
			     "bt_keys_get_addr() returned a non-valid reference");
	}
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Clear keys pool */
	clear_key_pool();
	/* Fill keys pool */
	keys_fill_pool();
}

void test_main(void)
{

	ztest_test_suite(bt_keys_update_usage_overwrite_oldest_enabled,
			 ztest_unit_test_setup(test_update_non_existing_key, unit_test_setup),
			 ztest_unit_test_setup(test_update_latest_reference, unit_test_setup),
			 ztest_unit_test_setup(test_update_non_latest_reference, unit_test_setup));

	ztest_run_test_suite(bt_keys_update_usage_overwrite_oldest_enabled);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
