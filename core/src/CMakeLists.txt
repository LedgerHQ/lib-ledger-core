cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
        async/LockedResource.cpp
        async/LockedResource.hpp
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
        crypto/AESCipher.hpp
        bitcoin/BitcoinLikeAddress.cpp bitcoin/BitcoinLikeAddress.hpp
        math/Base58.cpp math/Base58.hpp
        collections/vector.cpp collections/vector.hpp
        utils/Try.cpp utils/Try.hpp
        utils/Exception.cpp utils/Exception.hpp
        utils/djinni_helpers.cpp utils/djinni_helpers.hpp
        bitcoin/BitcoinLikeExtendedPublicKey.cpp bitcoin/BitcoinLikeExtendedPublicKey.hpp
        crypto/DeterministicPublicKey.cpp crypto/DeterministicPublicKey.hpp
        crypto/SECP256k1Point.cpp crypto/SECP256k1Point.hpp
        crypto/RIPEMD160.cpp crypto/RIPEMD160.hpp
        crypto/HMAC.cpp crypto/HMAC.hpp
        crypto/HASH160.cpp crypto/HASH160.hpp
        utils/DerivationPath.cpp utils/DerivationPath.hpp
        wallet/bitcoin/BitcoinLikeWallet.cpp wallet/bitcoin/BitcoinLikeWallet.hpp
        database/DatabaseBackend.cpp database/DatabaseBackend.hpp
        database/SQLite3Backend.cpp database/SQLite3Backend.hpp
        database/PostgreSQLBackend.cpp database/PostgreSQLBackend.hpp
        debug/LoggerApi.cpp debug/LoggerApi.hpp
        preferences/PreferencesBackend.cpp preferences/PreferencesBackend.hpp
        preferences/Preferences.cpp preferences/Preferences.hpp
        preferences/PreferencesEditor.cpp preferences/PreferencesEditor.hpp
        wallet/bitcoin/keychains/BitcoinLikeKeyChain.cpp wallet/bitcoin/keychains/BitcoinLikeKeyChain.hpp
        wallet/bitcoin/keychains/BitcoinLikeKeychain.cpp wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp
        wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.cpp wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp
        async/Future.cpp async/Future.hpp
        async/Deffered.cpp async/Deffered.hpp
        async/Promise.cpp async/Promise.hpp
        utils/Option.cpp utils/Option.hpp
        utils/ImmediateExecutionContext.cpp utils/ImmediateExecutionContext.hpp
        utils/Unit.cpp utils/Unit.hpp
        wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.cpp wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp
        wallet/bitcoin/networks.cpp wallet/bitcoin/networks.hpp
        collections/strings.cpp collections/strings.hpp
        io/filesystem.hpp io/filesystem.cpp wallet/bitcoin/factories/BitcoinLikeWalletFactory.cpp
        wallet/bitcoin/factories/BitcoinLikeWalletFactory.hpp bytes/serialization.cpp
        bytes/serialization.hpp async/DedicatedContext.cpp
        async/DedicatedContext.hpp traits/callback_traits.hpp
        config/Networks.cpp config/Networks.hpp
        net/HttpJsonHandler.cpp net/HttpJsonHandler.hpp
        net/HttpUrlConnectionInputStream.cpp net/HttpUrlConnectionInputStream.hpp
        utils/Either.cpp utils/Either.hpp
        utils/Lazy.cpp utils/Lazy.hpp
        bitcoin/BitcoinLikeBase58Provider.cpp bitcoin/BitcoinLikeBase58Provider.hpp
        bitcoin/BitcoinLikeExtendedPublicKeyProvider.cpp bitcoin/BitcoinLikeExtendedPublicKeyProvider.hpp
        async/CompletionBlock.cpp async/CompletionBlock.hpp
        traits/shared_ptr_traits.hpp collections/collections.hpp
        collections/MapLike.hpp utils/optional.cpp collections/DynamicObject.cpp collections/DynamicObject.hpp
        collections/DynamicArray.cpp collections/DynamicArray.hpp collections/DynamicValue.cpp
        collections/DynamicValue.hpp collections/Sequence.hpp collections/String.hpp wallet/bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.cpp wallet/bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.hpp net/URI.cpp net/URI.hpp collections/Bytes.hpp wallet/bitcoin/explorers/api/TransactionParser.cpp wallet/bitcoin/explorers/api/TransactionParser.hpp wallet/bitcoin/explorers/api/BlockParser.cpp wallet/bitcoin/explorers/api/BlockParser.hpp utils/DateParser.cpp utils/DateParser.hpp wallet/bitcoin/explorers/api/InputParser.cpp wallet/bitcoin/explorers/api/InputParser.hpp wallet/bitcoin/explorers/api/OutputParser.cpp wallet/bitcoin/explorers/api/OutputParser.hpp wallet/bitcoin/explorers/api/TransactionsParser.cpp wallet/bitcoin/explorers/api/TransactionsParser.hpp wallet/bitcoin/explorers/api/TransactionsBulkParser.cpp wallet/bitcoin/explorers/api/TransactionsBulkParser.hpp wallet/bitcoin/explorers/api/LedgerApiParser.hpp database/DatabaseSessionPool.cpp database/DatabaseSessionPool.hpp utils/DerivationScheme.cpp utils/DerivationScheme.hpp collections/functional.hpp)
link_directories(${CMAKE_BINARY_DIR}/lib)

target_link_libraries(ledger-core bigd)
target_link_libraries(ledger-core fmt)
target_link_libraries(ledger-core crypto)
target_link_libraries(ledger-core sqlite3)
target_link_libraries(ledger-core soci_sqlite3)
target_link_libraries(ledger-core soci_core_static)
target_link_libraries(ledger-core leveldb)

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
target_include_directories(ledger-core PUBLIC ../lib/soci_sqlite3)
target_include_directories(ledger-core PUBLIC ../lib/spdlog/include)
target_include_directories(ledger-core PUBLIC ../lib/openssl/include)
target_include_directories(ledger-core PUBLIC ../lib/leveldb/include)
target_include_directories(ledger-core PUBLIC ../test/lib/boost)
target_include_directories(ledger-core PUBLIC ../lib/cereal/)

install(TARGETS ledger-core DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core")