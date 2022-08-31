/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <mocks/crypto.h>

DEFINE_FFF_GLOBALS;

DEFINE_FAKE_VALUE_FUNC(int, bt_rand, void *, size_t);
