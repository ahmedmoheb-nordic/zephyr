/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/net/buf.h>
#include <zephyr/bluetooth/buf.h>
#include <mocks/net_buf.h>

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VOID_FUNC(net_buf_unref, struct net_buf *);
DEFINE_FAKE_VALUE_FUNC(void *, net_buf_simple_add_mem, struct net_buf_simple *, const void *,
			size_t);
