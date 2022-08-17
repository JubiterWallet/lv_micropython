# This top-level micropython.cmake is responsible for listing
# the individual modules we want to include.
# Paths are absolute, and ${CMAKE_CURRENT_LIST_DIR} can be
# used to prefix subdirectories.

add_compile_definitions(USE_NEM=1)
add_compile_definitions(USE_MONERO=1)
add_compile_definitions(BITCOIN_ONLY=0)
add_compile_definitions(SECP256K1_ZKP=True)
add_compile_definitions(RDI=True)

# Add the C example.
include(${CMAKE_CURRENT_LIST_DIR}/embed/extmod/modtrezorio/modtrezorio.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/extmod/modtrezorconfig/modtrezorconfig.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/extmod/modtrezorcrypto/modtrezorcrypto.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/extmod/trezorobj.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/extmod/modtrezorutils/modtrezorutils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/firmware/firmware.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vendor/storage/storage.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/embed/trezorhal/trezorhal.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vendor/trezor-crypto/crypto.cmake)


# Add the CPP example.
#include(${CMAKE_CURRENT_LIST_DIR}/extmod/micropython.cmake)

