/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <common/log.h>
#include <addr.h>

/* BT (ID, Address) pair */
struct id_addr_pair {
	uint8_t id;			    /*	ID */
	bt_addr_le_t *addr;		/*	Pointer to the address	*/
};

/* keys.c declarations */

struct bt_keys *bt_keys_get_key_pool(void);

/* keys_help_utils.c declarations */
bool check_key_pool_is_empty(void);
const char *bt_addr_str_real(const bt_addr_t *addr);
const char *bt_addr_le_str_real(const bt_addr_le_t *addr);

/* Repeat test entries */
#define REGISTER_SETUP_TEARDOWN(i, ...) \
	ztest_unit_test_setup_teardown(__VA_ARGS__, unit_test_setup, unit_test_noop)

#define ztest_unit_test_setup(fn, setup) \
	ztest_unit_test_setup_teardown(fn, setup, unit_test_noop)
