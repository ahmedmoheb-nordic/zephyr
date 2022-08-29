#
# Common include directories and source files for bluetooth/host/keys.c unit tests
#

include_directories(
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys
)

SET( host_module
  ${ZEPHYR_BASE}/subsys/bluetooth/host/keys.c
  ${ZEPHYR_BASE}/subsys/bluetooth/common/addr.c
)

SET( module_mocks
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/id.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/rpa.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/conn.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/hci_core.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/hci_core_expects.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/keys/mocks/keys_help_utils.c
)
