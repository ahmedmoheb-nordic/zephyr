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
 *  Clear an existing key (ID = 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID is set to 0
 *
 *  Expected behaviour:
 *   - The key content is cleared and removed from persistent memory
 */
void test_clear_key_with_id_equal_0(void)
{
	struct bt_keys empty_key;
	struct bt_keys *returned_key, *find_returned_ref;
	uint8_t id = BT_ADDR_ID_0;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	memset(&empty_key, 0x00, sizeof(struct bt_keys));

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	/* Request to clear the key */
	bt_keys_clear(returned_key);

	/* Verify that memory was cleared */
	zassert_mem_equal(returned_key, &empty_key, sizeof(struct bt_keys),
			  "Key content wasn't cleared by 'bt_keys_clear()'");

	/* Ensure that item doesn't exist in the keys pool after calling bt_keys_clear() */
	find_returned_ref = bt_keys_find_addr(id, addr);
	zassert_true(find_returned_ref == NULL,
		     "bt_keys_find_addr() returned a non-NULL reference");

	expect_not_called_u8_to_dec();
	expect_single_call_bt_settings_encode_key_with_null_key(&returned_key->addr);
	expect_single_call_settings_delete();
}

/*
 *  Clear an existing key (ID != 0) and verify the result
 *
 *  Constraints:
 *   - Key reference points to a valid item
 *   - Item ID isn't set to 0
 *
 *  Expected behaviour:
 *   - The key content is cleared and removed from persistent memory
 */
void test_clear_key_with_id_not_equal_0(void)
{
	struct bt_keys empty_key;
	struct bt_keys *returned_key, *find_returned_ref;
	uint8_t id = BT_ADDR_ID_1;
	bt_addr_le_t *addr = BT_ADDR_LE_1;

	memset(&empty_key, 0x00, sizeof(struct bt_keys));

	/* Add custom item to the keys pool */
	returned_key = bt_keys_get_addr(id, addr);
	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a non-valid reference");

	/* Request to clear the key */
	bt_keys_clear(returned_key);

	/* Verify that memory was cleared */
	zassert_mem_equal(returned_key, &empty_key, sizeof(struct bt_keys),
			  "Key content wasn't cleared by 'bt_keys_clear()'");

	/* Ensure that item doesn't exist in the keys pool after calling bt_keys_clear() */
	find_returned_ref = bt_keys_find_addr(id, addr);
	zassert_true(find_returned_ref == NULL,
		     "bt_keys_find_addr() returned a non-NULL reference");

	expect_single_call_u8_to_dec(id);
	expect_single_call_bt_settings_encode_key_with_not_null_key(&returned_key->addr);
	expect_single_call_settings_delete();
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

static bool pragma_bt_settings_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_SETTINGS))) {
		return true;
	}

	return false;
}

ztest_register_test_suite(
	bt_keys_clear_bt_settings_enabled, pragma_bt_settings_enabled,
	ztest_unit_test_setup(test_clear_key_with_id_equal_0, unit_test_setup),
	ztest_unit_test_setup(test_clear_key_with_id_not_equal_0, unit_test_setup));
