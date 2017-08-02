
add_library(
        wally-core
        STATIC
        include/wally_bip32.h
        include/wally_core.h
        include/wally_bip38.h
        include/wally_bip39.h
        include/wally_elements.h
        include/wally_crypto.h

        src/aes.c
        src/base58.c
        src/bip32.c
        src/bip38.c
        src/elements.c
        src/hex.c
        src/bip39.c
        src/hmac.c
        src/internal.c
        src/mnemonic.c
        src/pbkdf2.c
        src/scrypt.c
        src/sign.c
        src/wordlist.c


)

target_include_directories(wally-core PUBLIC .)
target_include_directories(wally-core PUBLIC src)
target_include_directories(wally-core PUBLIC src/ccan)