/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include "kconfig.h"
#include <keys.h>
#include "host_mocks/assert.h"
#include "mocks/keys_help_utils.h"

/* This LUT contains different combinations of ID, Address and key type.
 * Item in this list will be used to fill keys pool.
 */
static const struct id_addr_type testing_id_addr_type_lut[CONFIG_BT_MAX_PAIRED] = {
	{ BT_ADDR_ID_1, BT_ADDR_LE_1, BT_KEYS_PERIPH_LTK },
	{ BT_ADDR_ID_1, BT_ADDR_LE_2, BT_KEYS_IRK },
	{ BT_ADDR_ID_2, BT_ADDR_LE_1, BT_KEYS_LTK },
	{ BT_ADDR_ID_2, BT_ADDR_LE_2, BT_KEYS_LOCAL_CSRK },
	{ BT_ADDR_ID_3, BT_ADDR_LE_1, BT_KEYS_REMOTE_CSRK },
	{ BT_ADDR_ID_3, BT_ADDR_LE_2, BT_KEYS_LTK_P256 },
	{ BT_ADDR_ID_4, BT_ADDR_LE_1, BT_KEYS_ALL }
};

/* This LUT contains different combinations of ID, Address and key type.
 * Item in this list won't be used to fill keys pool.
 */
static const struct id_addr_type non_existing_id_addr_type_lut[] = {
	{ BT_ADDR_ID_1, BT_ADDR_LE_5, BT_KEYS_PERIPH_LTK },
	{ BT_ADDR_ID_1, BT_ADDR_LE_1, BT_KEYS_LOCAL_CSRK },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_IRK },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_LTK },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_LOCAL_CSRK },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_REMOTE_CSRK },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_LTK_P256 },
	{ BT_ADDR_ID_5, BT_ADDR_LE_5, BT_KEYS_ALL },
};

/* Global iterator to iterate over the ID, Address and type LUT */
static uint8_t params_it;

/* Pointer to the current set of testing parameters */
struct id_addr_type const *current_params_vector;

/* This list will hold returned references while filling keys pool */
static struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_type_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_type_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Test function with non-existing item
 *
 *  Constraints:
 *   - Valid values of non-existing items are used
 *
 *  Expected behaviour:
 *   - NULL reference is returned
 */
void test_find_non_existing_item(void)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(non_existing_id_addr_type_lut); i++) {
		struct bt_keys *returned_ref;
		struct id_addr_type const *params_vector = &non_existing_id_addr_type_lut[i];
		int type = params_vector->type;
		uint8_t id = params_vector->id;
		const bt_addr_le_t *addr = params_vector->addr;

		returned_ref = bt_keys_find(type, id, addr);
		zassert_true(returned_ref == NULL, "bt_keys_find() returned a non-NULL reference");
	}
}

/*
 *  Test function with existing item
 *
 *  Constraints:
 *   - Keys pool list is filled
 *   - Valid values of existing items are used
 *
 *  Expected behaviour:
 *   - Valid reference pointer is returned and matches the correct reference
 */
void test_find_existing_item(void)
{
	struct bt_keys *returned_ref, *expected_key_ref;
	int type = current_params_vector->type;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_ref = bt_keys_find(type, id, addr);
	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	zassert_true(returned_ref != NULL, "bt_keys_find() returned a NULL reference");
	zassert_equal_ptr(returned_ref, expected_key_ref,
			  "bt_keys_find() returned unexpected reference");
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
	struct id_addr_type const *params_vector;

	zassert_true(check_key_pool_is_empty(),
		     "List isn't empty, make sure to run this test just after a fresh start");

	for (size_t it = 0; it < ARRAY_SIZE(testing_id_addr_type_lut); it++) {
		params_vector = &testing_id_addr_type_lut[it];
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
		returned_keys_refs[i]->type |= testing_id_addr_type_lut[i].type;
	}
}

/* Setup test variables */
static void unit_test_setup(void)
{
	zassert_true((params_it < (ARRAY_SIZE(testing_id_addr_type_lut))),
		     "Invalid testing parameters index %u", params_it);
	current_params_vector = &testing_id_addr_type_lut[params_it];
	params_it++;
}

void test_main(void)
{

	ztest_test_suite(bt_keys_find_item_empty_list,
			 ztest_unit_test(test_find_non_existing_item));

	ztest_run_test_suite(bt_keys_find_item_empty_list);

	/* Setup functions */
	clear_key_pool();
	keys_fill_pool();
	set_keys_type();
	reset_global_test_round_params();

	ztest_test_suite(bt_keys_find_item_filled_list,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_existing_item),
			 ztest_unit_test(test_find_non_existing_item));

	ztest_run_test_suite(bt_keys_find_item_filled_list);

	uint32_t state;

	ztest_run_registered_test_suites(&state);
}
