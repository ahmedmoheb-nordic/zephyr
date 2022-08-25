/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "mocks/util.h"
#include "mocks/util_expects.h"
#include "mocks/settings.h"
#include "mocks/settings_store.h"
#include "mocks/settings_expects.h"
#include "mocks/settings_store_expects.h"
#include "mocks/keys_help_utils.h"

/*
 *  Store an existing key (ID = 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID is set to 0
 *   - Return value from settings_save_one() is 0
 *
 *  Expected behaviour:
 *   - Return value is 0
 */
void test_id_equal_0_with_no_error(void)
{
	int returned_code;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_0;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	settings_save_one_fake.return_val = 0;

	/* Store the key */
	returned_code = bt_keys_store(returned_key);

	zassert_true(returned_code == 0, "bt_keys_store() returned a non-zero code");

	expect_not_called_u8_to_dec();
	expect_single_call_bt_settings_encode_key_with_null_key(&returned_key->addr);
	expect_single_call_settings_save_one(returned_key->storage_start);
}

/*
 *  Store an existing key (ID = 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID is set to 0
 *   - Return value from settings_save_one() is -1
 *
 *  Expected behaviour:
 *   - Return value is -1
 */
void test_id_equal_0_with_error(void)
{
	int returned_code;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_0;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	settings_save_one_fake.return_val = -1;

	/* Store the key */
	returned_code = bt_keys_store(returned_key);

	zassert_true(returned_code == -1, "bt_keys_store() returned a non-zero code");

	expect_not_called_u8_to_dec();
	expect_single_call_bt_settings_encode_key_with_null_key(&returned_key->addr);
	expect_single_call_settings_save_one(returned_key->storage_start);
}

/*
 *  Store an existing key (ID != 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID isn't set to 0
 *   - Return value from settings_save_one() is 0
 *
 *  Expected behaviour:
 *   - Return value is 0
 */
void test_id_not_equal_0_with_no_error(void)
{
	int returned_code;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_1;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	settings_save_one_fake.return_val = 0;

	/* Store the key */
	returned_code = bt_keys_store(returned_key);

	zassert_true(returned_code == 0, "bt_keys_store() returned a non-zero code");

	expect_single_call_u8_to_dec(id);
	expect_single_call_bt_settings_encode_key_with_not_null_key(&returned_key->addr);
	expect_single_call_settings_save_one(returned_key->storage_start);
}

/*
 *  Store an existing key (ID != 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID isn't set to 0
 *   - Return value from settings_save_one() is -1
 *
 *  Expected behaviour:
 *   - Return value is -1
 */
void test_id_not_equal_0_with_error(void)
{
	int returned_code;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_1;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	settings_save_one_fake.return_val = -1;

	/* Store the key */
	returned_code = bt_keys_store(returned_key);

	zassert_true(returned_code == -1, "bt_keys_store() returned a non-zero code");

	expect_single_call_u8_to_dec(id);
	expect_single_call_bt_settings_encode_key_with_not_null_key(&returned_key->addr);
	expect_single_call_settings_save_one(returned_key->storage_start);
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Clear keys pool */
	clear_key_pool();

	/* Register resets */
	UTIL_FFF_FAKES_LIST(RESET_FAKE);
	SETTINGS_FFF_FAKES_LIST(RESET_FAKE);
	SETTINGS_STORE_FFF_FAKES_LIST(RESET_FAKE);
}

void test_main(void)
{
	ztest_test_suite(
		bt_keys_store_key_bt_settings_enabled,
		ztest_unit_test_setup(test_id_equal_0_with_no_error, unit_test_setup),
		ztest_unit_test_setup(test_id_equal_0_with_error, unit_test_setup),
		ztest_unit_test_setup(test_id_not_equal_0_with_no_error, unit_test_setup),
		ztest_unit_test_setup(test_id_not_equal_0_with_error, unit_test_setup));

	ztest_run_test_suite(bt_keys_store_key_bt_settings_enabled);

	uint32_t state = 0;

	ztest_run_registered_test_suites(&state);
}
