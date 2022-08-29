/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "mocks/hci_core_expects.h"

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST isn't enabled
 *
 *  Expected behaviour:
 *   - NULL reference pointer is returned
 */
void test_adding_new_pair_to_full_list(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_3;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_3);

	returned_key = bt_keys_get_addr(id, &addr);

	expect_not_called_bt_unpair();

	zassert_true(returned_key == NULL, "bt_keys_get_addr() returned a non-NULL reference");
}

static bool pragma_overwrite_not_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST))) {
		return false;
	}

	return true;
}

ztest_register_test_suite(bt_keys_get_addr_full_list_no_overwrite, pragma_overwrite_not_enabled,
			  ztest_unit_test(test_adding_new_pair_to_full_list));
