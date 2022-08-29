/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "host_mocks/assert.h"
#include "mocks/conn.h"
#include "mocks/hci_core.h"
#include "mocks/keys_help_utils.h"
#include "mocks/hci_core_expects.h"

/* This call back will be called when bt_unpair() is called with the same parameters
 * It can be used when a call to bt_unpair() isn't expected
 */
static int bt_unpair_unreachable_custom_fake(uint8_t id, const bt_addr_le_t *addr)
{
	ARG_UNUSED(id);
	ARG_UNUSED(addr);
	zassert_unreachable("Unexpected call to 'bt_unpair()' occurred");
	return 0;
}

/* This call back will be called when bt_conn_foreach() is called with the same parameters */
static void bt_conn_foreach_custom_fake_conn_ref_null(int type, bt_conn_foreach_cb func,
							void *data)
{
	expect_assert();
	func(NULL, data);
}

/* This call back will be called when bt_conn_foreach() is called with the same parameters */
static void bt_conn_foreach_custom_fake_data_ref_null(int type, bt_conn_foreach_cb func,
							void *data)
{
	struct bt_conn conn;

	expect_assert();
	func(&conn, NULL);
}

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - All Keys are used with a connection
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST is enabled
 *
 *  Expected behaviour:
 *   - An assertion is triggered and execution stops
 */
void test_find_key_in_use_receives_null_conn_ref(void)
{
	bt_addr_le_t addr;
	uint8_t id = BT_ADDR_ID_5;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_5);

	bt_unpair_fake.custom_fake = bt_unpair_unreachable_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_conn_ref_null;

	bt_keys_get_addr(id, &addr);

	/* Should not reach this point */
	zassert_unreachable(NULL);
}

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - All Keys are used with a connection
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST is enabled
 *
 *  Expected behaviour:
 *   - An assertion is triggered and execution stops
 */
void test_find_key_in_use_receives_null_data_ref(void)
{
	bt_addr_le_t addr;
	uint8_t id = BT_ADDR_ID_5;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_5);

	bt_unpair_fake.custom_fake = bt_unpair_unreachable_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_data_ref_null;

	bt_keys_get_addr(id, &addr);

	/* Should not reach this point */
	zassert_unreachable(NULL);
}

/*
 *  Test invalid (NULL) BT address reference
 *
 *  Constraints:
 *   - Address value is NULL
 *
 *  Expected behaviour:
 *   - An assertion is raised and execution stops
 */
void test_null_address_reference(void)
{
	expect_assert();
	bt_keys_get_addr(0x00, NULL);
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Register resets */
	ASSERT_FFF_FAKES_LIST(RESET_FAKE);
	CONN_FFF_FAKES_LIST(RESET_FAKE);
	HCI_CORE_FFF_FAKES_LIST(RESET_FAKE);
}

static bool pragma_overwrite_enabled(const void *s)
{
	if ((IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST))) {
		return true;
	}

	return false;
}

ztest_register_test_suite(bt_keys_get_addr_null_reference, NULL,
			  ztest_unit_test_setup(test_null_address_reference, unit_test_setup));

ztest_register_test_suite(
	bt_keys_find_key_in_use_invalid_cases, pragma_overwrite_enabled,
	ztest_unit_test_setup(test_find_key_in_use_receives_null_conn_ref, unit_test_setup),
	ztest_unit_test_setup(test_find_key_in_use_receives_null_data_ref, unit_test_setup));
