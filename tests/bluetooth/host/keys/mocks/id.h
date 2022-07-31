/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/fff.h>
#include <addr.h>
#include <keys.h>

/* List of fakes used by this unit tester */
#define ID_FFF_FAKES_LIST(FAKE)        \
		FAKE(bt_id_del)                \

/* void bt_id_del(struct bt_keys *keys) mock */
DECLARE_FAKE_VOID_FUNC(bt_id_del, struct bt_keys *);
