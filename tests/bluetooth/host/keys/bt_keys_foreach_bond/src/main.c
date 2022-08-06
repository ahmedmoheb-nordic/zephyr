/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include "kconfig.h"
#include <addr.h>
#include <keys.h>
#include "host_mocks/assert.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID and Address pairs */
static const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED] = {
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
static struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_pair_lut) == ARRAY_SIZE(returned_keys_refs));

/* Callback to be used when no calls are expected by bt_foreach_bond() */
static void bt_foreach_bond_unreachable_cb(const struct bt_bond_info *info, void *user_data)
{
	zassert_unreachable("Unexpected call to '%s()' occurred", __func__);
}

/* Callback to be used when calls are expected by bt_foreach_bond() */
static void bt_foreach_bond_expected_cb(const struct bt_bond_info *info, void *user_data)
{
	uint32_t *call_counter = (uint32_t *)user_data;

	zassert_true(info != NULL, "Unexpected NULL reference pointer for parameter '%s'", "info");
	zassert_true(user_data != NULL, "Unexpected NULL reference pointer for parameter '%s'",
		     "user_data");

	(*call_counter)++;
}

/*
 *  Test calling bt_foreach_bond() with a valid ID that exists
 *  in the keys pool but the keys type isn't set
 *
 *  Constraints:
 *   - Keys pool has been filled
 *   - Keys type isn't set
 *
 *  Expected behaviour:
 *   - Callback should never be called
 */
void test_existing_id_type_is_not_set(void)
{
	uint8_t id = current_params_vector->id;
	void *user_data = (void *)current_params_vector;

	bt_foreach_bond(id, bt_foreach_bond_unreachable_cb, (void *)user_data);
}

/*
 *  Test calling bt_foreach_bond() with a valid ID that exists
 *  in the keys pool while the keys type is set
 *
 *  Constraints:
 *   - Keys pool has been filled
 *   - Keys type is set
 *
 *  Expected behaviour:
 *   - Callback should be called for each occurrence
 */
void test_existing_id_type_is_set(void)
{
	uint32_t call_counter = 0;
	uint8_t id = current_params_vector->id;
	/* Each ID was registered in the list with 2 different addresses */
	bt_foreach_bond(id, bt_foreach_bond_expected_cb, (void *)&call_counter);
	zassert_true(call_counter == 2,
		     "Incorrect call counter for 'bt_foreach_bond_expected_cb()'");
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

/* Set keys type */
static void set_keys_type(void)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(returned_keys_refs); i++) {
		returned_keys_refs[i]->type |= BT_KEYS_ALL;
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

	clear_key_pool();
	keys_fill_pool();
	reset_global_test_round_params();

	/* Testing if a valid ID is used, but the keys type isn't set */
	ztest_test_suite(bt_keys_foreach_bond_valid_inputs_type_not_set,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_existing_id_type_is_not_set));

	ztest_run_test_suite(bt_keys_foreach_bond_valid_inputs_type_not_set);

	set_keys_type();
	reset_global_test_round_params();

	/* Testing if a valid ID is used while the keys type is set */
	ztest_test_suite(bt_keys_foreach_bond_valid_inputs_type_set,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_existing_id_type_is_set));

	ztest_run_test_suite(bt_keys_foreach_bond_valid_inputs_type_set);

	uint32_t state;

	ztest_run_registered_test_suites(&state);
}
