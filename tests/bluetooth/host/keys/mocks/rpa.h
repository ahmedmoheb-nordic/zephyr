/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/fff.h>
#include <addr.h>

/* List of fakes used by this unit tester */
#define RPA_FFF_FAKES_LIST(FAKE)       \
		FAKE(bt_rpa_irk_matches)       \

/* bool bt_rpa_irk_matches(const uint8_t irk[16], const bt_addr_t *addr) mock */
DECLARE_FAKE_VALUE_FUNC(bool, bt_rpa_irk_matches, const uint8_t *, const bt_addr_t *);
