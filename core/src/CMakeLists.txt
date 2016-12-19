
list(APPEND ledger-core-sources
        bytes/BytesReader.cpp
        ledger-core.h
        math/BigInt.h
        math/BigInt.cpp

        utils/endian.cpp
        utils/endian.h
        utils/hex.cpp
        utils/optional.hpp

        bytes/BytesWriter.cpp
        bytes/BytesWriter.h
        async/ExecutionContext.hpp
        async/Callback.hpp
        async/EventEmitter.hpp
        api/BigInt.hpp

        api_impl/LedgerCoreImpl.cpp
        api_impl/LedgerCoreImpl.hpp
        )

set(ANDROID_CPP_FEATURES exceptions)

# Automaticall ADD djinni api files to the library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
file(GLOB ledger-core-djinni-api-sources
        "api/*"
        )
list(APPEND ledger-core-sources ${ledger-core-djinni-api-sources})

if (TARGET_JNI)
    message(STATUS "Configure project for JNI")
    #find_package(JNI REQUIRED)
    file(GLOB ledger-core-jni-sources
            "jni/*.cpp"
            "jni/*.hpp"
            "jni/jni/*"
            )
    list(APPEND ledger-core-sources ${ledger-core-jni-sources})
endif ()

add_library(
        ledger-core SHARED
        ${ledger-core-sources}
        api_impl/BigIntImpl.cpp
        api_impl/BigIntImpl.hpp
        wallet/pool/WalletPool.cpp
        wallet/pool/WalletPool.hpp
        wallet/pool/WalletPoolBuilder.cpp
        wallet/pool/WalletPoolBuilder.hpp
        utils/LambdaRunnable.cpp
        utils/LambdaRunnable.hpp
        preferences/JsonPreferences.cpp
        preferences/JsonPreferences.hpp
        preferences/IPreferencesBackend.hpp
        preferences/AtomicPreferencesBackend.cpp
        preferences/AtomicPreferencesBackend.hpp
        async/LockedResource.cpp
        async/LockedResource.hpp
        preferences/JsonPreferencesEditor.cpp
        preferences/JsonPreferencesEditor.hpp
        debug/logger.cpp
        debug/logger.hpp
        debug/RotatingEncryptableSink.cpp
        debug/RotatingEncryptableSink.hpp
        debug/LogPrinterSink.cpp
        debug/LogPrinterSink.hpp
        debug/logger.hpp debug/logger.cpp
        net/HttpClient.cpp
        net/HttpClient.hpp
        crypto/SHA256.cpp
        crypto/SHA256.hpp
        crypto/AES256.cpp
        crypto/AES256.hpp
        crypto/PBKDF2.cpp
        crypto/PBKDF2.hpp
        crypto/AESCipher.cpp
        crypto/AESCipher.hpp bitcoin/BitcoinLikeAddress.cpp bitcoin/BitcoinLikeAddress.hpp math/Base58.cpp math/Base58.hpp utils/vector.cpp utils/vector.hpp utils/Try.cpp utils/Try.hpp utils/Exception.cpp utils/Exception.hpp utils/djinni_helpers.cpp utils/djinni_helpers.hpp bitcoin/BitcoinLikeExtendedPublicKey.cpp bitcoin/BitcoinLikeExtendedPublicKey.hpp crypto/DeterministicPublicKey.cpp crypto/DeterministicPublicKey.hpp crypto/SECP256k1Point.cpp crypto/SECP256k1Point.hpp crypto/RIPEMD160.cpp crypto/RIPEMD160.hpp crypto/HMAC.cpp crypto/HMAC.hpp crypto/HASH160.cpp crypto/HASH160.hpp utils/DerivationPath.cpp utils/DerivationPath.hpp)



target_link_libraries(ledger-core bigd)
target_link_libraries(ledger-core fmt)
target_link_libraries(ledger-core crypto)

file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        "*.h"
        )

file(GLOB LEDGER_CORE_API_HEADERS
        "api/*.hpp"
        )

include_directories(ledger-core ../lib/bigd)

if (TARGET_JNI)
    # include_directories(ledger-core ${JNI_INCLUDE_DIRS})
    # target_link_libraries(ledger-core ${JNI_LIBRARIES})
endif ()

file(GLOB ledger-core-header
        "*"
        )
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.h")
file(COPY ${ledger-core-header} DESTINATION ${CMAKE_BINARY_DIR}/include/ledger/core FILES_MATCHING PATTERN "*.hpp")

target_include_directories(ledger-core PUBLIC ..)
target_include_directories(ledger-core PUBLIC ../lib/bigd)
target_include_directories(ledger-core PUBLIC ../lib/fmt-3.0.0)
target_include_directories(ledger-core PUBLIC ../lib/rapidjson/include)
target_include_directories(ledger-core PUBLIC ../lib/soci/core)
target_include_directories(ledger-core PUBLIC ../lib/spdlog/include)
target_include_directories(ledger-core PUBLIC ../lib/openssl/include)

install(TARGETS ledger-core DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core")