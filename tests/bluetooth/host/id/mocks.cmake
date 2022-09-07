#
# Common include directories and source files for bluetooth/host/id.c unit tests
#

include_directories(
  ${ZEPHYR_BASE}/tests/bluetooth/host/id
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks
)

SET( host_module
  ${ZEPHYR_BASE}/subsys/bluetooth/host/id.c
  ${ZEPHYR_BASE}/subsys/bluetooth/common/addr.c
)

SET( module_mocks
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks/scan.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks/addr.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks/crypto.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks/net_buf.c
  ${ZEPHYR_BASE}/tests/bluetooth/host/id/mocks/hci_core.c
)
