/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mocks/adv.h"

#include <zephyr/kernel.h>

DEFINE_FAKE_VALUE_FUNC(int, bt_le_adv_set_enable, struct bt_le_ext_adv *, bool);
