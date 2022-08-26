/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include <keys.h>
#include "host_mocks/assert.h"
#include "mocks/keys_help_utils.h"

/*
 *  Test function with NULL address
 *
 *  Constraints:
 *   - Any ID value can be used
 *   - Address is passed as NULL
 *
 *  Expected behaviour:
 *   - An assertion is raised and execution stops
 */
void test_null_device_address(void)
{
	expect_assert();
	bt_keys_update_usage(0x00, NULL);
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Register resets */
	ASSERT_FFF_FAKES_LIST(RESET_FAKE);
}

static bool pragma_bt_overwrite_oldest_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST))) {
		return true;
	}

	return false;
}

ztest_register_test_suite(bt_keys_update_usage_invalid_inputs, pragma_bt_overwrite_oldest_enabled,
			  ztest_unit_test_setup(test_null_device_address, unit_test_setup));
