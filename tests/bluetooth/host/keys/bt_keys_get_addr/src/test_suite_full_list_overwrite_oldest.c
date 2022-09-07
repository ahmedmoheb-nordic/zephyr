/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include "testing_common_defs.h"
#include <addr.h>
#include <keys.h>
#include "mocks/conn.h"
#include "mocks/hci_core.h"
#include "mocks/keys_help_utils.h"
#include "mocks/hci_core_expects.h"

/* This LUT contains different combinations of ID and Address pairs */
extern const struct id_addr_pair testing_id_addr_pair_lut[CONFIG_BT_MAX_PAIRED];

/* This list holds returned references while filling keys pool */
extern struct bt_keys *returned_keys_refs[CONFIG_BT_MAX_PAIRED];

/* Pointer to the current set of oldest testing parameter */
struct id_addr_pair const *oldest_params_vector;

/*
 * Update the 'oldest_params_vector' value to point to the oldest set.
 * It returns the index of the oldest parameters
 */
static uint32_t update_oldest_params_vector(void)
{
	static uint32_t oldest_params_idx;

	zassert_true((oldest_params_idx < (ARRAY_SIZE(testing_id_addr_pair_lut))),
		     "Invalid testing parameters index %u", oldest_params_idx);

	oldest_params_vector = &testing_id_addr_pair_lut[oldest_params_idx++];
	if (oldest_params_idx == (ARRAY_SIZE(testing_id_addr_pair_lut))) {
		oldest_params_idx = 0;
	}

	return (oldest_params_vector - &testing_id_addr_pair_lut[0]);
}

/* This call back will be called when bt_unpair() is called with the same parameters
 * It will clear the key slot with the assigned to the 'id'
 */
static int bt_unpair_custom_fake(uint8_t id, const bt_addr_le_t *addr)
{
	struct bt_keys *keys = NULL;

	keys = bt_keys_find_addr(id, addr);
	if (keys) {
		bt_keys_clear(keys);
	}

	/* This check is used here because bt_unpair() is called with a local variable address */
	expect_single_call_bt_unpair(oldest_params_vector->id, oldest_params_vector->addr);

	return 0;
}

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

/* This call back will be called when bt_conn_foreach() is called with the same parameters
 * It will act as if there are some active connections
 */
static void bt_conn_foreach_custom_fake_key_slot_0_in_use(int type, bt_conn_foreach_cb func,
							  void *data)
{
	struct bt_conn conn;

	/* This will make the effect as if there is a disconnection */
	conn.state = BT_CONN_DISCONNECTED;
	func(&conn, data);

	/* This will make the effect as if there is a connection with no key */
	conn.state = BT_CONN_CONNECTED;
	conn.id = 0xFF;
	bt_addr_le_copy(&conn.le.dst, (const bt_addr_le_t *)BT_ADDR_LE_ANY);
	bt_conn_get_dst_fake.return_val = &conn.le.dst;
	func(&conn, data);

	/* This will make the effect as if key at slot 0 is in use with a connection */
	conn.state = BT_CONN_CONNECTED;
	conn.id = testing_id_addr_pair_lut[0].id;
	bt_addr_le_copy(&conn.le.dst, testing_id_addr_pair_lut[0].addr);
	bt_conn_get_dst_fake.return_val = &conn.le.dst;
	func(&conn, data);
}

/* This call back will be called when bt_conn_foreach() is called with the same parameters
 * It will act as if all keys are in used with active connections
 */
static void bt_conn_foreach_custom_fake_all_keys_in_use(int type, bt_conn_foreach_cb func,
							void *data)
{
	struct bt_conn conn;

	/* This will make the effect as if key at slot 'x' is in use with a connection */
	for (size_t i = 0; i < ARRAY_SIZE(testing_id_addr_pair_lut); i++) {
		conn.state = BT_CONN_CONNECTED;
		conn.id = testing_id_addr_pair_lut[i].id;
		bt_addr_le_copy(&conn.le.dst, testing_id_addr_pair_lut[i].addr);
		bt_conn_get_dst_fake.return_val = &conn.le.dst;
		func(&conn, data);
	}
}

/* This call back will be called when bt_conn_foreach() is called with the same parameters
 * It will act as if all keys are in used with active connections
 */
static void bt_conn_foreach_custom_fake_no_keys_in_use(int type, bt_conn_foreach_cb func,
							void *data)
{
	struct bt_conn conn;

	/* This will make the effect as if key at slot 'x' is in use with a connection */
	for (size_t i = 0; i < ARRAY_SIZE(testing_id_addr_pair_lut); i++) {
		conn.state = BT_CONN_DISCONNECTED;
		conn.id = testing_id_addr_pair_lut[i].id;
		bt_addr_le_copy(&conn.le.dst, testing_id_addr_pair_lut[i].addr);
		bt_conn_get_dst_fake.return_val = &conn.le.dst;
		func(&conn, data);
	}
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
 *   - NULL pointer is returned as there is no room
 */
void test_add_item_full_list_all_keys_in_use(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_3;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_3);

	bt_unpair_fake.custom_fake = bt_unpair_unreachable_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_all_keys_in_use;

	returned_key = bt_keys_get_addr(id, &addr);

	zassert_true(returned_key == NULL, "bt_keys_get_addr() returned a non-NULL reference");
}

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - All Keys are not used with a connection
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST is enabled
 *
 *  Expected behaviour:
 *   - Valid reference pointer is returned which matches the oldest
 *     reference at index 1
 */
void test_add_item_full_list_no_keys_in_use(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_3;
	uint32_t oldest_params_ref_idx;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_3);

	oldest_params_ref_idx = update_oldest_params_vector();
	bt_unpair_fake.custom_fake = bt_unpair_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_no_keys_in_use;

	returned_key = bt_keys_get_addr(id, &addr);

	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a NULL reference");
	zassert_true(returned_key == returned_keys_refs[oldest_params_ref_idx],
		     "bt_keys_get_addr() returned reference doesn't match expected one");
}

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - Oldest key at slot 0 is used with a connection
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST is enabled
 *
 *  Expected behaviour:
 *   - Valid reference pointer is returned which matches the oldest
 *     reference at index 1
 */
void test_add_item_full_list_key_0_in_use_key_1_oldest(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_4;
	uint32_t oldest_params_ref_idx;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_4);

	bt_unpair_fake.custom_fake = bt_unpair_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_key_slot_0_in_use;

	oldest_params_ref_idx = update_oldest_params_vector();

	returned_key = bt_keys_get_addr(id, &addr);

	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a NULL reference");
	zassert_true(returned_key == returned_keys_refs[oldest_params_ref_idx],
		     "bt_keys_get_addr() returned reference doesn't match expected one");
}

/*
 *  Test adding extra (ID, Address) pair while the keys pool list is full
 *
 *  Constraints:
 *   - Keys pool list is full
 *   - Key at slot 0 is used with a connection
 *   - Oldest key isn't used with a connection
 *   - CONFIG_BT_KEYS_OVERWRITE_OLDEST is enabled
 *
 *  Expected behaviour:
 *   - Valid reference pointer is returned which matches the oldest
 *     reference at index 2
 */
void test_add_item_full_list_key_0_in_use_key_2_oldest(void)
{
	bt_addr_le_t addr;
	struct bt_keys *returned_key;
	uint8_t id = BT_ADDR_ID_5;
	uint32_t oldest_params_ref_idx;

	bt_addr_le_copy(&addr, (const bt_addr_le_t *)BT_ADDR_LE_5);

	oldest_params_ref_idx = update_oldest_params_vector();
	bt_unpair_fake.custom_fake = bt_unpair_custom_fake;
	bt_conn_foreach_fake.custom_fake = bt_conn_foreach_custom_fake_key_slot_0_in_use;

	returned_key = bt_keys_get_addr(id, &addr);

	zassert_true(returned_key != NULL, "bt_keys_get_addr() returned a NULL reference");
	zassert_true(returned_key == returned_keys_refs[oldest_params_ref_idx],
		     "bt_keys_get_addr() returned reference doesn't match expected one");
}

/* Setup test variables */
static void unit_test_setup(void)
{
	/* Register resets */
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

/* This test suite verifies the order at which the oldest key is overwritten.
 * So, the output is dependent on the order of execution.
 */
ztest_register_test_suite(
	bt_keys_get_addr_full_list_overwrite_oldest, pragma_overwrite_enabled,
	ztest_unit_test_setup(test_add_item_full_list_all_keys_in_use, unit_test_setup),
	ztest_unit_test_setup(test_add_item_full_list_no_keys_in_use, unit_test_setup),
	ztest_unit_test_setup(test_add_item_full_list_key_0_in_use_key_1_oldest, unit_test_setup),
	ztest_unit_test_setup(test_add_item_full_list_key_0_in_use_key_2_oldest, unit_test_setup));
