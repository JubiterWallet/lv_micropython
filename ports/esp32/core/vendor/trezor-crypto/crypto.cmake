# Create an INTERFACE library for our C module.
add_library(usermod_crypto INTERFACE)

# Add our source files to the lib
target_sources(usermod_crypto INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/address.c 
    ${CMAKE_CURRENT_LIST_DIR}/aes/aes_modes.c 
    ${CMAKE_CURRENT_LIST_DIR}/aes/aescrypt.c 
    ${CMAKE_CURRENT_LIST_DIR}/aes/aeskey.c 
    ${CMAKE_CURRENT_LIST_DIR}/aes/aestab.c 
    ${CMAKE_CURRENT_LIST_DIR}/base32.c 
    ${CMAKE_CURRENT_LIST_DIR}/base58.c 
    ${CMAKE_CURRENT_LIST_DIR}/bignum.c 
    ${CMAKE_CURRENT_LIST_DIR}/bip32.c 
    ${CMAKE_CURRENT_LIST_DIR}/bip39.c 
    ${CMAKE_CURRENT_LIST_DIR}/bip39_english.c 
    ${CMAKE_CURRENT_LIST_DIR}/blake256.c 
    ${CMAKE_CURRENT_LIST_DIR}/blake2b.c 
    ${CMAKE_CURRENT_LIST_DIR}/blake2s.c 
    ${CMAKE_CURRENT_LIST_DIR}/chacha20poly1305/chacha20poly1305.c 
    ${CMAKE_CURRENT_LIST_DIR}/chacha20poly1305/chacha_merged.c 
    ${CMAKE_CURRENT_LIST_DIR}/chacha20poly1305/poly1305-donna.c 
    ${CMAKE_CURRENT_LIST_DIR}/chacha20poly1305/rfc7539.c 
    ${CMAKE_CURRENT_LIST_DIR}/chacha_drbg.c 
    ${CMAKE_CURRENT_LIST_DIR}/curves.c 
    ${CMAKE_CURRENT_LIST_DIR}/ecdsa.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/curve25519-donna-32bit.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/curve25519-donna-helpers.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/curve25519-donna-scalarmult-base.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519-donna-32bit-tables.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519-donna-basepoint-table.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519-donna-impl-base.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519-keccak.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519-sha3.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/ed25519.c 
    ${CMAKE_CURRENT_LIST_DIR}/ed25519-donna/modm-donna-32bit.c 
    ${CMAKE_CURRENT_LIST_DIR}/groestl.c 
    ${CMAKE_CURRENT_LIST_DIR}/hasher.c 
    ${CMAKE_CURRENT_LIST_DIR}/hmac.c 
    ${CMAKE_CURRENT_LIST_DIR}/hmac_drbg.c 
    ${CMAKE_CURRENT_LIST_DIR}/memzero.c 
    ${CMAKE_CURRENT_LIST_DIR}/nem.c 
    ${CMAKE_CURRENT_LIST_DIR}/nist256p1.c 
    ${CMAKE_CURRENT_LIST_DIR}/pbkdf2.c 
    ${CMAKE_CURRENT_LIST_DIR}/rand.c 
    ${CMAKE_CURRENT_LIST_DIR}/rfc6979.c 
    ${CMAKE_CURRENT_LIST_DIR}/ripemd160.c 
    ${CMAKE_CURRENT_LIST_DIR}/secp256k1.c 
    ${CMAKE_CURRENT_LIST_DIR}/sha2.c 
    ${CMAKE_CURRENT_LIST_DIR}/sha3.c 
    ${CMAKE_CURRENT_LIST_DIR}/shamir.c 
    ${CMAKE_CURRENT_LIST_DIR}/slip39.c 
	${CMAKE_CURRENT_LIST_DIR}/cardano.c
	${CMAKE_CURRENT_LIST_DIR}/monero/base58.c
	${CMAKE_CURRENT_LIST_DIR}/monero/serialize.c
	${CMAKE_CURRENT_LIST_DIR}/monero/xmr.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_crypto INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)




# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_crypto)
