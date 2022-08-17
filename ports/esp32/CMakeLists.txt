# Top-level cmake file for building MicroPython on ESP32.

cmake_minimum_required(VERSION 3.12)

add_compile_definitions(BITCOIN_ONLY=0)
add_definitions(-DAES_128)
add_definitions(-DAES_192)
add_definitions(-DRAND_PLATFORM_INDEPENDENT)
add_definitions(-DRANDI)
add_compile_definitions(USE_BIP32_CACHE=0)
add_compile_definitions(USE_KECCAK=1)
add_compile_definitions(USE_ETHEREUM=1)
add_compile_definitions(USE_MONERO=1)
add_compile_definitions(USE_CARDANO=1)
add_compile_definitions(USE_NEM=1)
add_compile_definitions(USE_EOS=1)
add_compile_definitions(TREZOR_MODEL_T)

# Set the location of this port's directory.
set(MICROPY_PORT_DIR ${CMAKE_SOURCE_DIR})

# Set the board if it's not already set.
if(NOT MICROPY_BOARD)
    set(MICROPY_BOARD GENERIC)
endif()

# Set the board directory and check that it exists.
if(NOT MICROPY_BOARD_DIR)
    set(MICROPY_BOARD_DIR ${MICROPY_PORT_DIR}/boards/${MICROPY_BOARD})
endif()
if(NOT EXISTS ${MICROPY_BOARD_DIR}/mpconfigboard.cmake)
    message(FATAL_ERROR "Invalid MICROPY_BOARD specified: ${MICROPY_BOARD}")
endif()

# Include main IDF cmake file.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the output sdkconfig so it goes in the build directory.
set(SDKCONFIG ${CMAKE_BINARY_DIR}/sdkconfig)

# Save the manifest file set from the cmake command line.
set(MICROPY_USER_FROZEN_MANIFEST ${MICROPY_FROZEN_MANIFEST})

# Include board config; this is expected to set SDKCONFIG_DEFAULTS (among other options).
include(${MICROPY_BOARD_DIR}/mpconfigboard.cmake)

# Set the frozen manifest file. Note if MICROPY_FROZEN_MANIFEST is set from the cmake
# command line, then it will override the default and any manifest set by the board.
if (MICROPY_USER_FROZEN_MANIFEST)
    set(MICROPY_FROZEN_MANIFEST ${MICROPY_USER_FROZEN_MANIFEST})
elseif (NOT MICROPY_FROZEN_MANIFEST)
    set(MICROPY_FROZEN_MANIFEST ${MICROPY_PORT_DIR}/boards/manifest.py)
endif()

# Add sdkconfig fragments that depend on the IDF version.
if(IDF_VERSION_MAJOR EQUAL 4 AND IDF_VERSION_MINOR LESS 2)
    set(SDKCONFIG_DEFAULTS ${SDKCONFIG_DEFAULTS} boards/sdkconfig.nimble_core0)
else()
    set(SDKCONFIG_DEFAULTS ${SDKCONFIG_DEFAULTS} boards/sdkconfig.nimble_core1)
endif()

# Concatenate all sdkconfig files into a combined one for the IDF to use.
file(WRITE ${CMAKE_BINARY_DIR}/sdkconfig.combined.in "")
foreach(SDKCONFIG_DEFAULT ${SDKCONFIG_DEFAULTS})
    file(READ ${SDKCONFIG_DEFAULT} CONTENTS)
    file(APPEND ${CMAKE_BINARY_DIR}/sdkconfig.combined.in "${CONTENTS}")
endforeach()
configure_file(${CMAKE_BINARY_DIR}/sdkconfig.combined.in ${CMAKE_BINARY_DIR}/sdkconfig.combined COPYONLY)
set(SDKCONFIG_DEFAULTS ${CMAKE_BINARY_DIR}/sdkconfig.combined)

# Include LVGL component, ignore KCONFIG

idf_build_set_property(LV_MICROPYTHON 1)
idf_build_component(${CMAKE_SOURCE_DIR}/../../lib/lv_bindings/lvgl)
# Build Rust component
idf_build_component(${CMAKE_SOURCE_DIR}/embed/rust/clib)
idf_build_component(${CMAKE_SOURCE_DIR}/embed/rust/rustlib)
idf_build_set_property(COMPILE_DEFINITIONS "-DLV_KCONFIG_IGNORE" APPEND)
separate_arguments(LV_CFLAGS_ENV UNIX_COMMAND $ENV{LV_CFLAGS})
list(APPEND LV_CFLAGS ${LV_CFLAGS_ENV})
idf_build_set_property(COMPILE_DEFINITIONS "${LV_CFLAGS}" APPEND)

# Define the project.
project(micropython)


