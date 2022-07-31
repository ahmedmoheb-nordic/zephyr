/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <addr.h>
#include <host/keys.h>
#include "kconfig.h"
#include "keys_help_utils.h"

bool check_key_pool_is_empty(void)
{
	int i;
	struct bt_keys *keys, *key_pool;

	key_pool = bt_keys_get_key_pool();
	for (i = 0; i < CONFIG_BT_MAX_PAIRED; i++) {
		keys = &key_pool[i];
		if (bt_addr_le_cmp(&keys->addr, BT_ADDR_LE_ANY)) {
			return false;
		}
	}

	return true;
}

static int bt_addr_le_to_str_mod(const bt_addr_le_t *addr, char *str,
					size_t len)
{
	char type[10];

	switch (addr->type) {
	case BT_ADDR_LE_PUBLIC:
		strcpy(type, "public");
		break;
	case BT_ADDR_LE_RANDOM:
		strcpy(type, "random");
		break;
	case BT_ADDR_LE_PUBLIC_ID:
		strcpy(type, "public-id");
		break;
	case BT_ADDR_LE_RANDOM_ID:
		strcpy(type, "random-id");
		break;
	default:
		snprintf(type, sizeof(type), "0x%02x", addr->type);
		break;
	}

	return snprintf(str, len, "%02X:%02X:%02X:%02X:%02X:%02X (%s)",
			addr->a.val[5], addr->a.val[4], addr->a.val[3],
			addr->a.val[2], addr->a.val[1], addr->a.val[0], type);
}

static int bt_addr_to_str_mod(const bt_addr_t *addr, char *str, size_t len)
{
	return snprintf(str, len, "%02X:%02X:%02X:%02X:%02X:%02X",
			addr->val[5], addr->val[4], addr->val[3],
			addr->val[2], addr->val[1], addr->val[0]);
}

const char *bt_addr_str_real(const bt_addr_t *addr)
{
	static char str[BT_ADDR_STR_LEN];

	bt_addr_to_str_mod(addr, str, sizeof(str));

	return str;
}

const char *bt_addr_le_str_real(const bt_addr_le_t *addr)
{
	static char str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str_mod(addr, str, sizeof(str));

	return str;
}
