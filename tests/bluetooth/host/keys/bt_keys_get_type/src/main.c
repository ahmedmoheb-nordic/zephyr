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

/* Global iterator to iterate over the ID, Address and type LUT */
static uint8_t params_it;

/* Pointer to the current set of testing parameters */
struct id_addr_type const *current_params_vector;

/* This list will hold returned references while filling keys pool */
static struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_type_lut) == CONFIG_BT_MAX_PAIRED);
BUILD_ASSERT(ARRAY_SIZE(testing_id_addr_type_lut) == ARRAY_SIZE(returned_keys_refs));

/*
 *  Test getting a non-existing key reference with type, ID and Address while
 *  the list isn't full
 *
 *  Constraints:
 *   - Empty keys pool list
 *
 *  Expected behaviour:
 *   - A key slot is reserved and data type, ID and Address are stored
 *   - A valid reference is returned by bt_keys_get_type()
 *   - ID value matches the one passed to bt_keys_get_type()
 *   - Address value matches the one passed to bt_keys_get_type()
 *   - Key type value matches the one passed to bt_keys_get_type()
 */
void test_get_non_existing_key_reference(void)
{
	struct bt_keys *returned_key;
	int type = current_params_vector->type;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_key = bt_keys_get_type(type, id, addr);
	/* 'params_it' is incremented before running the test case */
	returned_keys_refs[params_it - 1] = returned_key;

	zassert_true(returned_key != NULL, "bt_keys_get_type() returned a non-valid reference");
	zassert_true(returned_key->id == id,
		     "bt_keys_get_type() returned a reference with an incorrect ID");
	zassert_true(returned_key->type == type,
		     "bt_keys_get_type() returned a reference with an incorrect key type");
	zassert_true(!bt_addr_le_cmp(&returned_key->addr, addr),
		     "bt_keys_get_type() returned incorrect address %s value, expected %s",
		     bt_addr_le_str(&returned_key->addr), bt_addr_le_str(addr));
}

/*
 *  Test getting a non-existing key reference with type, ID and Address while
 *  the list is full
 *
 *  Constraints:
 *   - Full keys pool list
 *
 *  Expected behaviour:
 *   - A NULL value is returned by bt_keys_get_type()
 */
void test_get_non_existing_key_reference_full_list(void)
{
	struct bt_keys *returned_key;
	int type = BT_KEYS_IRK;
	uint8_t id = BT_ADDR_ID_5;
	const bt_addr_le_t *addr = BT_ADDR_LE_5;

	returned_key = bt_keys_get_type(type, id, addr);

	zassert_true(returned_key == NULL, "bt_keys_get_type() returned a non-NULL reference");
}

/*
 *  Test getting an existing key reference with type, ID and Address while
 *  the list is full
 *
 *  Constraints:
 *   - Full keys pool list
 *
 *  Expected behaviour:
 *   - A valid reference is returned by bt_keys_get_type()
 *   - Key reference returned matches the previously returned one
 *     when it was firstly inserted in the the list
 */
void test_get_existing_key_reference(void)
{
	struct bt_keys *returned_key, *expected_key_ref;
	int type = current_params_vector->type;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_key = bt_keys_get_type(type, id, addr);
	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	zassert_true(returned_key != NULL, "bt_keys_get_type() returned a non-valid reference");
	zassert_equal_ptr(returned_key, expected_key_ref,
			  "bt_keys_get_type() returned unexpected reference");
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
	zassert_true((params_it < (ARRAY_SIZE(testing_id_addr_type_lut))),
		     "Invalid testing parameters index %u", params_it);
	current_params_vector = &testing_id_addr_type_lut[params_it];
	params_it++;
}

void test_main(void)
{

	reset_global_test_round_params();

	ztest_test_suite(bt_keys_get_type_non_existing_key,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_get_non_existing_key_reference),
			 ztest_unit_test(test_get_non_existing_key_reference_full_list));

	ztest_run_test_suite(bt_keys_get_type_non_existing_key);

	reset_global_test_round_params();

	ztest_test_suite(bt_keys_get_type_existing_key,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_get_existing_key_reference));

	ztest_run_test_suite(bt_keys_get_type_existing_key);

	uint32_t state;

	ztest_run_registered_test_suites(&state);
}
