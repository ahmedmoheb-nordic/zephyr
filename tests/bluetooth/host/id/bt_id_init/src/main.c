/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <bluetooth/bluetooth.h>
#include <keys.h>
#include <id.h>
#include "host_mocks/assert.h"

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
	bt_id_init();
}

void test_main(void)
{

	ztest_test_suite(bt_keys_find_item_empty_list,
			 ztest_unit_test(test_find_non_existing_item));

	ztest_run_test_suite(bt_keys_find_item_empty_list);
}
