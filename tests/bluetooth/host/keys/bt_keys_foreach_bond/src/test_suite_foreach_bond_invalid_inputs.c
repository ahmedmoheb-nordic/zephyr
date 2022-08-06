/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include "host_mocks/assert.h"
#include "mocks/keys_help_utils.h"

/* Callback to be used when no calls are expected by bt_foreach_bond() */
static void bt_foreach_bond_unreachable_cb(const struct bt_bond_info *info, void *user_data)
{
	zassert_unreachable("Unexpected call to '%s()' occurred", __func__);
}

/*
 *  Test callback function is set to NULL
 *
 *  Constraints:
 *   - Any ID value can be used
 *   - Callback function pointer is set to NULL
 *
 *  Expected behaviour:
 *   - An assertion is raised and execution stops
 */
void test_null_callback(void)
{
	expect_assert();
	bt_foreach_bond(0x00, NULL, NULL);
}

/*
 *  Test empty list with non-existing ID
 *
 *  Constraints:
 *   - ID doesn't exist in the list
 *   - Valid callback is passed to bt_keys_foreach_bond()
 *
 *  Expected behaviour:
 *   - Callback should never be called
 */
void test_empty_list_with_non_existing_id(void)
{
	bt_foreach_bond(0x00, bt_foreach_bond_unreachable_cb, NULL);
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Register resets */
	ASSERT_FFF_FAKES_LIST(RESET_FAKE);
}

ztest_register_test_suite(bt_keys_foreach_bond_invalid_inputs, NULL,
			  ztest_unit_test_setup(test_null_callback, unit_test_setup),
			  ztest_unit_test(test_empty_list_with_non_existing_id));
