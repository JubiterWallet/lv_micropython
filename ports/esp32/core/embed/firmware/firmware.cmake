# Create an INTERFACE library for our C module.
add_library(usermod_firmware INTERFACE)

# Add our source files to the lib
target_sources(usermod_firmware INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/delay.c 
#    ${CMAKE_CURRENT_LIST_DIR}/mphalport.c        
)

# Add the current directory as an include directory.
target_include_directories(usermod_firmware INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_firmware)
