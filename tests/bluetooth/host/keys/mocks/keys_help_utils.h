/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <common/log.h>
#include <addr.h>

/* keys.c declarations */

struct bt_keys *bt_keys_get_key_pool(void);

/* keys_help_utils.c declarations */
bool check_key_pool_is_empty(void);
const char *bt_addr_str_real(const bt_addr_t *addr);
const char *bt_addr_le_str_real(const bt_addr_le_t *addr);
