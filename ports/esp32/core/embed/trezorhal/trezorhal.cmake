# Create an INTERFACE library for our C module.
add_library(usermod_trezorhal INTERFACE)

# Add our source files to the lib
target_sources(usermod_trezorhal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/flash.c
    ${CMAKE_CURRENT_LIST_DIR}/random_delays.c        
    ${CMAKE_CURRENT_LIST_DIR}/image.c   
    ${CMAKE_CURRENT_LIST_DIR}/usb.c  
)

# Add the current directory as an include directory.
target_include_directories(usermod_trezorhal INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_trezorhal)
