/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Common Kconfig settings for bluetooth/host mocks */

#define CONFIG_BT_ID_MAX 2
#define CONFIG_BT_MAX_PAIRED 1

/** Bluetooth LE device "test" addresses */
#define BT_ADDR_LE_TEST1 ((bt_addr_le_t[]) { { 0, { { 0x01, 0x23, 0x45, 0x67, 0x89, 0x0A } } } })
#define BT_ADDR_LE_TEST2 ((bt_addr_le_t[]) { { 0, { { 0x01, 0x23, 0x45, 0x67, 0x89, 0x0B } } } })
#define BT_ADDR_LE_TEST3 ((bt_addr_le_t[]) { { 0, { { 0x01, 0x23, 0x45, 0x67, 0x89, 0x0C } } } })

/* Assertion configuration */

#define CONFIG_ASSERT 1
#define CONFIG_ASSERT_LEVEL 2
#define CONFIG_ASSERT_VERBOSE 1
