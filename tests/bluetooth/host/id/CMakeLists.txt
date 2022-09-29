#
# CMakeLists.txt file for creating of mocks library.
#

add_library(mocks STATIC
            mocks/scan.c
            mocks/addr.c
            mocks/addr_expects.c
            mocks/rpa.c
            mocks/adv.c
            mocks/smp.c
            mocks/conn.c
            mocks/keys.c
            mocks/keys_expects.c
            mocks/kernel.c
            mocks/kernel_expects.c
            mocks/crypto.c
            mocks/crypto_expects.c
            mocks/settings.c
            mocks/settings_expects.c
            mocks/net_buf.c
            mocks/net_buf_expects.c
            mocks/hci_core.c
            mocks/hci_core_expects.c

            ${ZEPHYR_BASE}/subsys/bluetooth/host/id.c
            ${ZEPHYR_BASE}/subsys/bluetooth/common/addr.c
            ${ZEPHYR_BASE}/subsys/logging/log_minimal.c
            ${ZEPHYR_BASE}/subsys/bluetooth/common/bt_str.c
            ${ZEPHYR_BASE}/subsys/bluetooth/host/uuid.c
)

target_include_directories(mocks PUBLIC
  .
  ${ZEPHYR_BASE}/subsys/bluetooth
  ${ZEPHYR_BASE}/subsys/bluetooth/host
  ${ZEPHYR_BASE}/tests/bluetooth/host
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks
)

target_link_libraries(mocks PRIVATE test_interface)
