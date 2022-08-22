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
static const struct id_addr_type testing_id_addr_type_lut[CONFIG_BT_MAX_PAIRED] = {

	{ BT_ADDR_ID_1, BT_ADDR_LE_1, BT_KEYS_PERIPH_LTK },
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_1, BT_KEYS_PERIPH_LTK },
	{ BT_ADDR_ID_1, BT_RPA_ADDR_LE_2, BT_KEYS_IRK },
	{ BT_ADDR_ID_1, BT_ADDR_LE_3, BT_KEYS_IRK },

	{ BT_ADDR_ID_2, BT_ADDR_LE_1, BT_KEYS_LTK },
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_2, BT_KEYS_IRK },
	{ BT_ADDR_ID_2, BT_RPA_ADDR_LE_3, BT_KEYS_IRK },
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

/* Check if a Bluetooth LE random address is resolvable private address. */
#define BT_ADDR_IS_RPA(a)     (((a)->val[5] & 0xc0) == 0x40)

static bool check_if_addr_is_rpa(const bt_addr_le_t *addr)
{
	if (addr->type != BT_ADDR_LE_RANDOM) {
		return false;
	}

	return BT_ADDR_IS_RPA(&addr->a);
}


/* This call back will be called when bt_rpa_irk_matches() is called with the same parameters
 * It can be used when a call to bt_rpa_irk_matches() isn't expected
 */
static bool bt_rpa_irk_matches_unreachable_custom_fake(const uint8_t irk[16], const bt_addr_t *addr)
{
	ARG_UNUSED(irk);
	ARG_UNUSED(addr);
	zassert_unreachable("Unexpected call to 'bt_rpa_irk_matches()' occurred");
	return true;
}

/* This call back will be called when bt_rpa_irk_matches() is called with the same parameters
 * It can be used when a call to bt_rpa_irk_matches() isn't expected
 */
static bool bt_rpa_irk_matches_custom_fake(const uint8_t irk[16], const bt_addr_t *addr)
{
	/* Check if the address matches the current testing vector device address */
	if (irk[0] != (params_it - 1) && !bt_addr_cmp(&current_params_vector->addr->a, addr)) {
		return false;
	}

	return true;
}

/*
 *  Find a non-existing key reference for ID and Address of type 'BT_KEYS_IRK'
 *
 *  Constraints:
 *   - Empty keys pool list
 *
 *  Expected behaviour:
 *   - A NULL value is returned
 */
void test_find_non_existing_key_reference(void)
{
	struct bt_keys *returned_ref;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	returned_ref = bt_keys_find_irk(id, addr);

	zassert_true(returned_ref == NULL, "bt_keys_find_irk() returned a non-valid reference");
}

/*
 *  Find an existing key reference for ID and Address of type 'BT_KEYS_IRK'
 *  while the IRK value doesn't match device address.
 *
 *  Constraints:
 *   - Full keys pool list
 *   - IRK value and device address don't match
 *
 *  Expected behaviour:
 *   - A NULL value is returned
 */
void test_find_existing_key_reference_irk_and_address_not_matched(void)
{
	struct bt_keys *returned_ref;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	bt_rpa_irk_matches_fake.return_val = false;

	returned_ref = bt_keys_find_irk(id, addr);

	zassert_true(returned_ref == NULL, "bt_keys_find_irk() returned a non-valid reference");
}

/*
 *  Find an existing key reference for ID and Address of type 'BT_KEYS_IRK'
 *  while the IRK value and device address match
 *
 *  Constraints:
 *   - Full keys pool list
 *   - IRK value and device address match
 *
 *  Expected behaviour:
 *   - A valid reference value is returned
 */
void test_find_existing_key_reference_irk_val_and_input_address_match(void)
{
	struct bt_keys *returned_ref, *expected_key_ref;
	int type = current_params_vector->type;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	bt_rpa_irk_matches_fake.custom_fake = bt_rpa_irk_matches_custom_fake;

	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	/*
	 * Try to resolve the current testing vector address.
	 * Address will be considered resolvable if:
	 *  - The current testing vector address is an RPA
	 *  - The current testing vector key type is an IRK
	 */
	returned_ref = bt_keys_find_irk(id, addr);

	if (check_if_addr_is_rpa(addr) && (type & BT_KEYS_IRK)) {

		zassert_true(returned_ref != NULL, "bt_keys_find_irk() returned a NULL reference");
		zassert_equal_ptr(returned_ref, expected_key_ref,
				  "bt_keys_find_irk() returned unexpected reference");

		/* Check if address has been stored */
		zassert_mem_equal(&returned_ref->irk.rpa, &addr->a, sizeof(bt_addr_t),
				  "Incorrect address was stored by 'bt_keys_find_irk()'");
	} else {
		zassert_true(returned_ref == NULL,
			     "bt_keys_find_irk() returned a non-valid reference");
	}
}

/*
 *  Find an existing key reference for ID and Address of type 'BT_KEYS_IRK'
 *  while the IRK address and device address match
 *
 *  Constraints:
 *   - Full keys pool list
 *   - IRK address and device address match
 *
 *  Expected behaviour:
 *   - A valid reference value is returned
 */
void test_find_existing_key_reference_irk_addr_and_input_address_match(void)
{
	struct bt_keys *returned_ref, *expected_key_ref;
	int type = current_params_vector->type;
	uint8_t id = current_params_vector->id;
	const bt_addr_le_t *addr = current_params_vector->addr;

	/*
	 * Now, as the address under test should have been resolved before, bt_rpa_irk_matches()
	 * isn't expected to be called for an RPA.
	 *
	 * But, for other records, which won't be resolved, a call to bt_rpa_irk_matches() is
	 * expected simulating the attempt to resolve it
	 */
	if (check_if_addr_is_rpa(addr) && (type & BT_KEYS_IRK)) {
		bt_rpa_irk_matches_fake.custom_fake = bt_rpa_irk_matches_unreachable_custom_fake;
	} else {
		bt_rpa_irk_matches_fake.custom_fake = bt_rpa_irk_matches_custom_fake;
	}

	/* 'params_it' is incremented before running the test case */
	expected_key_ref = returned_keys_refs[params_it - 1];

	returned_ref = bt_keys_find_irk(id, addr);

	if (check_if_addr_is_rpa(addr) && (type & BT_KEYS_IRK)) {

		zassert_true(returned_ref != NULL, "bt_keys_find_irk() returned a NULL reference");
		zassert_equal_ptr(returned_ref, expected_key_ref,
				  "bt_keys_find_irk() returned unexpected reference");

		/* Check if address has been stored */
		zassert_mem_equal(&returned_ref->irk.rpa, &addr->a, sizeof(bt_addr_t),
				  "Incorrect address was stored by 'bt_keys_find_irk()'");
	} else {
		zassert_true(returned_ref == NULL,
			     "bt_keys_find_irk() returned a non-valid reference");
	}
}

/* Initialize test variables */
static void reset_global_test_round_params(void)
{
	params_it = 0;
	current_params_vector = NULL;
}

/* Fill the keys pool with testing ID, Address and key type */
static void keys_fill_pool_and_type(void)
{
	uint8_t id;
	bt_addr_le_t *addr;
	int type;
	struct id_addr_type const *params_vector;

	zassert_true(check_key_pool_is_empty(),
		     "List isn't empty, make sure to run this test just after a fresh start");

	for (size_t it = 0; it < ARRAY_SIZE(testing_id_addr_type_lut); it++) {
		params_vector = &testing_id_addr_type_lut[it];
		id = params_vector->id;
		addr = params_vector->addr;
		type = params_vector->type;
		returned_keys_refs[it] = bt_keys_get_type(type, id, addr);
		zassert_true(returned_keys_refs[it] != NULL,
			     "bt_keys_get_type() returned a non-valid reference");

		/* Set the index to the IRK value to recognize during callback */
		returned_keys_refs[it]->irk.val[0] = it;
	}
}

/* Setup test variables */
static void unit_test_setup(void)
{
	zassert_true((params_it < (ARRAY_SIZE(testing_id_addr_type_lut))),
		     "Invalid testing parameters index %u", params_it);
	current_params_vector = &testing_id_addr_type_lut[params_it];
	params_it++;

	/* Register resets */
	RPA_FFF_FAKES_LIST(RESET_FAKE);
}

void test_main(void)
{

	reset_global_test_round_params();

	ztest_test_suite(bt_keys_find_irk_non_existing_key_empty_pool,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_non_existing_key_reference));

	ztest_run_test_suite(bt_keys_find_irk_non_existing_key_empty_pool);

	reset_global_test_round_params();
	clear_key_pool();
	keys_fill_pool_and_type();

	ztest_test_suite(bt_keys_find_irk_existing_key_no_matched_address,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_existing_key_reference_irk_and_address_not_matched));

	ztest_run_test_suite(bt_keys_find_irk_existing_key_no_matched_address);

	reset_global_test_round_params();

	ztest_test_suite(bt_keys_find_irk_existing_key_new_matched_address,
			 LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
				 test_find_existing_key_reference_irk_val_and_input_address_match));

	ztest_run_test_suite(bt_keys_find_irk_existing_key_new_matched_address);

	reset_global_test_round_params();

	ztest_test_suite(
		bt_keys_find_irk_existing_key_existing_matched_address,
		LISTIFY(CONFIG_BT_MAX_PAIRED, REGISTER_SETUP_TEARDOWN, (,),
			test_find_existing_key_reference_irk_addr_and_input_address_match));

	ztest_run_test_suite(bt_keys_find_irk_existing_key_existing_matched_address);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
