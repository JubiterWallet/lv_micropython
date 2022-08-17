# Create an INTERFACE library for our C module.
add_library(usermod_modtrezorcrypto INTERFACE)

# Add our source files to the lib
target_sources(usermod_modtrezorcrypto INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/crc.c    
    ${CMAKE_CURRENT_LIST_DIR}/modtrezorcrypto.c  
#    ${CMAKE_CURRENT_LIST_DIR}/rand.c    
)

# Add the current directory as an include directory.
target_include_directories(usermod_modtrezorcrypto INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_modtrezorcrypto)
