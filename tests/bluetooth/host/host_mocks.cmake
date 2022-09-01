#
# Common include directories and source files for bluetooth/host unit tests
#

include_directories(
  ${ZEPHYR_BASE}/subsys/bluetooth
  ${ZEPHYR_BASE}/tests/bluetooth/host
)

SET( host_mocks
  ${ZEPHYR_BASE}/tests/bluetooth/host/host_mocks/assert.c
)

add_definitions(-include ztest.h)
