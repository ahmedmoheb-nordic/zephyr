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
 *  Test function with NULL key reference
 *
 *  Constraints:
 *   - Key reference is passed as NULL
 *
 *  Expected behaviour:
 *   - An assertion is raised and execution stops
 */
void test_null_key_reference(void)
{
	expect_assert();
	bt_keys_store(NULL);
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Register resets */
	ASSERT_FFF_FAKES_LIST(RESET_FAKE);
}

static bool pragma_bt_settings_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_SETTINGS))) {
		return true;
	}

	return false;
}

ztest_register_test_suite(bt_keys_store_invalid_inputs, pragma_bt_settings_enabled,
			  ztest_unit_test_setup(test_null_key_reference, unit_test_setup));
