# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(SOURCES
  src/main.c
  src/test_suite_bt_settings.c
  src/test_suite_invalid_inputs.c
)

project(bt_id_delete)

# Suppress the format-zero-length error if GNUC is used
if (CMAKE_C_COMPILER_ID MATCHES "GNU")
add_compile_options(-Wno-error=format-zero-length -Wno-format-zero-length)
endif()

find_package(Zephyr COMPONENTS unittest HINTS $ENV{ZEPHYR_BASE})

add_subdirectory(${ZEPHYR_BASE}/tests/bluetooth/host host_mocks)
add_subdirectory(${ZEPHYR_BASE}/tests/bluetooth/host/id mocks)

target_link_libraries(testbinary PRIVATE mocks host_mocks)
