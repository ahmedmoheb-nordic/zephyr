/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <addr.h>

/* BT (ID, Address) pair */
struct id_addr_pair {
	uint8_t id;			    /* ID */
	bt_addr_le_t *addr;		/* Pointer to the address */
};

/* BT Key (ID, Address, type) info */
struct id_addr_type {
	uint8_t id;			    /*	ID */
	bt_addr_le_t *addr;		/*	Pointer to the address	*/
	int type;				/*	Key type */
};

/* keys.c declarations */
struct bt_keys *bt_keys_get_key_pool(void);
#if IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
struct bt_keys *bt_keys_get_last_keys_updated(void);
#endif

/* keys_help_utils.c declarations */
void clear_key_pool(void);
bool check_key_pool_is_empty(void);

/* Repeat test entries */
#define REGISTER_SETUP_TEARDOWN(i, ...) \
	ztest_unit_test_setup_teardown(__VA_ARGS__, unit_test_setup, unit_test_noop)

#define ztest_unit_test_setup(fn, setup) \
	ztest_unit_test_setup_teardown(fn, setup, unit_test_noop)
