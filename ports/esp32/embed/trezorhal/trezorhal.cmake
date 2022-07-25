# Create an INTERFACE library for our C module.
add_library(usermod_trezorhal INTERFACE)

# Add our source files to the lib
target_sources(usermod_trezorhal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/hal_usb.c
#    ${CMAKE_CURRENT_LIST_DIR}/modtrezorio_ext_flash.c
#    ${CMAKE_CURRENT_LIST_DIR}/modtrezorio.c        
)

# Add the current directory as an include directory.
target_include_directories(usermod_trezorhal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_trezorhal)
