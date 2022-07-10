/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/buf.h>
#include <host/hci_core.h>
#include "kconfig.h"
#include "buf_help_utils.h"

/*
 *  Return value from bt_buf_get_cmd_complete() should be NULL
 *
 *  This is to test the behaviour when memory allocation request fails
 *
 *  Constraints:
 *   - bt_dev.sent_cmd value is NULL
 *   - Timeout value is a positive non-zero value
 *   - net_buf_alloc() returns a NULL value
 *
 *  Expected behaviour:
 *   - net_buf_alloc() to be called with the correct memory allocation pool
 *     and the same timeout value passed to bt_buf_get_cmd_complete()
 *   - bt_dev.sent_cmd value isn't changed after calling bt_buf_get_cmd_complete()
 *   - bt_buf_get_cmd_complete() returns NULL
 */
void test_returns_null_sent_cmd_is_null(void)
{
	struct net_buf *returned_buf;
	struct net_buf *sent_cmd_not_changing_value;
	k_timeout_t timeout = Z_TIMEOUT_TICKS(1000);

	bt_dev.sent_cmd = NULL;
	sent_cmd_not_changing_value = bt_dev.sent_cmd;

	struct net_buf_pool *memory_pool;

	if ((IS_ENABLED(CONFIG_BT_HCI_ACL_FLOW_CONTROL))) {
		memory_pool = bt_buf_get_evt_pool();
	} else {
		memory_pool = bt_buf_get_hci_rx_pool();
	}

	ztest_expect_value(net_buf_alloc_fixed, pool, memory_pool);
	ztest_returns_value(net_buf_alloc_fixed, NULL);

	ztest_expect_value(net_buf_validate_timeout_value_mock, value, timeout.ticks);

	returned_buf = bt_buf_get_cmd_complete(timeout);

	zassert_is_null(returned_buf,
			"bt_buf_get_cmd_complete() returned non-NULL value while expecting NULL");

	zassert_equal(bt_dev.sent_cmd, sent_cmd_not_changing_value,
		     "bt_buf_get_cmd_complete() caused bt_dev.sent_cmd value to be changed");
}

/*
 *  Return value from bt_buf_get_cmd_complete() shouldn't be NULL
 *
 *  Constraints:
 *   - bt_dev.sent_cmd value is NULL
 *   - Timeout value is a positive non-zero value
 *   - net_buf_alloc() return a not NULL value
 *
 *  Expected behaviour:
 *   - net_buf_alloc() to be called with the correct memory allocation pool
 *     and the same timeout value passed to bt_buf_get_cmd_complete()
 *   - bt_dev.sent_cmd value isn't changed after calling bt_buf_get_cmd_complete()
 *   - bt_buf_get_cmd_complete() returns the same value returned by net_buf_alloc_fixed()
 *   - Return buffer type is set to BT_BUF_EVT
 */
void test_returns_not_null_sent_cmd_is_null(void)
{
	static struct net_buf expected_buf;
	struct net_buf *returned_buf;
	uint8_t returned_buffer_type;
	struct net_buf *sent_cmd_not_changing_value;
	k_timeout_t timeout = Z_TIMEOUT_TICKS(1000);

	bt_dev.sent_cmd = NULL;
	sent_cmd_not_changing_value = bt_dev.sent_cmd;

	struct net_buf_pool *memory_pool;

	if ((IS_ENABLED(CONFIG_BT_HCI_ACL_FLOW_CONTROL))) {
		memory_pool = bt_buf_get_evt_pool();
	} else {
		memory_pool = bt_buf_get_hci_rx_pool();
	}

	ztest_expect_value(net_buf_simple_reserve, buf, &expected_buf.b);
	ztest_expect_value(net_buf_simple_reserve, reserve, BT_BUF_RESERVE);

	ztest_expect_value(net_buf_alloc_fixed, pool, memory_pool);
	ztest_returns_value(net_buf_alloc_fixed, &expected_buf);

	ztest_expect_value(net_buf_validate_timeout_value_mock, value, timeout.ticks);

	returned_buf = bt_buf_get_cmd_complete(timeout);

	zassert_equal(returned_buf, &expected_buf,
		      "bt_buf_get_cmd_complete() returned incorrect buffer pointer value");

	returned_buffer_type = bt_buf_get_type(returned_buf);
	zassert_equal(returned_buffer_type, BT_BUF_EVT,
		      "bt_buf_get_cmd_complete() returned incorrect buffer type %u, expected %u (%s)",
		      returned_buffer_type, BT_BUF_EVT, STRINGIFY(BT_BUF_EVT));

	zassert_equal(bt_dev.sent_cmd, sent_cmd_not_changing_value,
		     "bt_buf_get_cmd_complete() caused bt_dev.sent_cmd value to be changed");
}

/*
 *  Return value from bt_buf_get_cmd_complete() shouldn't be NULL
 *
 *  Constraints:
 *   - bt_dev.sent_cmd value isn't NULL
 *   - Timeout value is a positive non-zero value
 *
 *  Expected behaviour:
 *   - net_buf_alloc() isn't called
 *   - bt_dev.sent_cmd value isn't changed after calling bt_buf_get_cmd_complete()
 *   - bt_buf_get_cmd_complete() returns the same value set to bt_dev.sent_cmd
 *   - Return buffer type is set to BT_BUF_EVT
 */
void test_returns_not_null_sent_cmd_is_not_null(void)
{
	static struct net_buf expected_buf;
	struct net_buf *returned_buf;
	uint8_t returned_buffer_type;
	struct net_buf *sent_cmd_not_changing_value;
	k_timeout_t timeout = Z_TIMEOUT_TICKS(1000);

	bt_dev.sent_cmd = &expected_buf;
	sent_cmd_not_changing_value = bt_dev.sent_cmd;

	ztest_expect_value(net_buf_simple_reserve, buf, &bt_dev.sent_cmd->b);
	ztest_expect_value(net_buf_simple_reserve, reserve, BT_BUF_RESERVE);
	ztest_expect_value(net_buf_ref, buf, bt_dev.sent_cmd);

	returned_buf = bt_buf_get_cmd_complete(timeout);

	zassert_equal(returned_buf, &expected_buf,
		      "bt_buf_get_cmd_complete() returned incorrect buffer pointer value");

	returned_buffer_type = bt_buf_get_type(returned_buf);
	zassert_equal(returned_buffer_type, BT_BUF_EVT,
		      "bt_buf_get_cmd_complete() returned incorrect buffer type %u, expected %u (%s)",
		      returned_buffer_type, BT_BUF_EVT, STRINGIFY(BT_BUF_EVT));

	zassert_equal(bt_dev.sent_cmd, sent_cmd_not_changing_value,
		     "bt_buf_get_cmd_complete() caused bt_dev.sent_cmd value to be changed");
}

void test_main(void)
{
	ztest_test_suite(bt_buf_get_cmd_complete_returns_not_null,
			ztest_unit_test(test_returns_not_null_sent_cmd_is_null),
			ztest_unit_test(test_returns_not_null_sent_cmd_is_not_null)
			);

	ztest_run_test_suite(bt_buf_get_cmd_complete_returns_not_null);

	ztest_test_suite(bt_buf_get_cmd_complete_returns_null,
			ztest_unit_test(test_returns_null_sent_cmd_is_null)
			);

	ztest_run_test_suite(bt_buf_get_cmd_complete_returns_null);
}
