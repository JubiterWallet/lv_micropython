# Create an INTERFACE library for our C module.
add_library(usermod_modtrezorutils INTERFACE)

# Add our source files to the lib
target_sources(usermod_modtrezorutils INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/modtrezorutils.c   
)

# Add the current directory as an include directory.
target_include_directories(usermod_modtrezorutils INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_modtrezorutils)
