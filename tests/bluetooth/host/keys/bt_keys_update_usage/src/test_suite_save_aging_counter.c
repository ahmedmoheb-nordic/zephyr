/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "mocks/settings_store.h"
#include "mocks/settings_store_expects.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID and Address pairs */
extern const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED];

/* This list holds returned references while filling keys pool */
extern struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

/*
 *  Request updating the latest key reference
 *
 *  Constraints:
 *   - Keys pool is filled with items
 *
 *  Expected behaviour:
 *   - Last updated key reference matches the last updated key reference
 *   - bt_keys_store() is called once with the correct parameters
 */
void test_update_usage_and_save_aging_counter(void)
{
	uint8_t id;
	uint32_t old_aging_counter;
	bt_addr_le_t *addr;
	struct bt_keys *expected_updated_keys;
	struct id_addr_pair const *params_vector;

	for (size_t it = 0; it < ARRAY_SIZE(testing_id_addr_pair_lut); it++) {
		params_vector = &testing_id_addr_pair_lut[it];
		id = params_vector->id;
		addr = params_vector->addr;
		expected_updated_keys = returned_keys_refs[it];
		old_aging_counter = expected_updated_keys->aging_counter;

		/* Reset fake functions call counter */
		SETTINGS_STORE_FFF_FAKES_LIST(RESET_FAKE);

		bt_keys_update_usage(id, addr);

		zassert_true(expected_updated_keys->aging_counter > (old_aging_counter),
			     "bt_keys_update_usage() set incorrect aging counter");

		zassert_equal_ptr(bt_keys_get_last_keys_updated(), expected_updated_keys,
			"bt_keys_update_usage() changed last updated key reference unexpectedly");

		/* Check if bt_keys_store() was called */
		expect_single_call_settings_save_one(expected_updated_keys->storage_start);
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

static bool pragma_saving_aging_counter_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING))) {
		return true;
	}

	return false;
}

/* This test suite verifies the order at which the oldest key is overwritten.
 * So, the output is dependent on the order of execution.
 */
ztest_register_test_suite(
	bt_keys_update_usage_save_aging_counter, pragma_saving_aging_counter_enabled,
	ztest_unit_test_setup(test_update_usage_and_save_aging_counter, unit_test_setup));
