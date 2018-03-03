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
    find_package(JNI REQUIRED)
    file(GLOB ledger-core-jni-sources
            "jni/*.cpp"
            "jni/*.hpp"
            "jni/jni/*"
            )
    list(APPEND ledger-core-sources ${ledger-core-jni-sources})
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

add_library(
        ledger-core SHARED
        ${ledger-core-sources}
        api_impl/BigIntImpl.cpp
        api_impl/BigIntImpl.hpp
        wallet/pool/api/WalletPoolApi.cpp
        wallet/pool/api/WalletPoolApi.hpp
        wallet/pool/api/WalletPoolBuilder.cpp
        wallet/pool/api/WalletPoolBuilder.hpp
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
        async/CompletionBlock.cpp async/CompletionBlock.hpp
        traits/shared_ptr_traits.hpp collections/collections.hpp
        collections/MapLike.hpp utils/optional.cpp collections/DynamicObject.cpp collections/DynamicObject.hpp
        collections/DynamicArray.cpp collections/DynamicArray.hpp collections/DynamicValue.cpp
        collections/DynamicValue.hpp collections/Sequence.hpp collections/String.hpp
        wallet/bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.cpp
        wallet/bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.hpp
        net/URI.cpp net/URI.hpp collections/Bytes.hpp wallet/bitcoin/explorers/api/TransactionParser.cpp
        wallet/bitcoin/explorers/api/TransactionParser.hpp wallet/bitcoin/explorers/api/BlockParser.cpp
        wallet/bitcoin/explorers/api/BlockParser.hpp utils/DateUtils.cpp utils/DateUtils.hpp
        wallet/bitcoin/explorers/api/InputParser.cpp wallet/bitcoin/explorers/api/InputParser.hpp
        wallet/bitcoin/explorers/api/OutputParser.cpp
        wallet/bitcoin/explorers/api/OutputParser.hpp
        wallet/bitcoin/explorers/api/TransactionsParser.cpp
        wallet/bitcoin/explorers/api/TransactionsParser.hpp
        wallet/bitcoin/explorers/api/TransactionsBulkParser.cpp
        wallet/bitcoin/explorers/api/TransactionsBulkParser.hpp
        wallet/bitcoin/explorers/api/LedgerApiParser.hpp
        database/DatabaseSessionPool.cpp database/DatabaseSessionPool.hpp
        utils/DerivationScheme.cpp utils/DerivationScheme.hpp collections/functional.hpp
        database/migrations.cpp database/migrations.hpp
        events/EventPublisher.cpp events/EventPublisher.hpp events/Event.cpp events/Event.hpp events/EventBus.cpp
        events/EventBus.hpp events/LambdaEventReceiver.cpp events/LambdaEventReceiver.hpp
        wallet/common/AbstractWallet.cpp wallet/common/AbstractWallet.hpp
        wallet/common/AbstractAccount.cpp wallet/common/AbstractAccount.hpp
        wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp wallet/bitcoin/BitcoinLikeAccount.cpp
        wallet/bitcoin/BitcoinLikeAccount.hpp wallet/bitcoin/observers/BitcoinLikeBlockchainObserver.hpp
        wallet/pool/WalletPool.cpp wallet/pool/WalletPool.hpp wallet/pool/CurrenciesManager.cpp
        wallet/pool/CurrenciesManager.hpp wallet/currencies.cpp wallet/currencies.hpp wallet/common/CurrencyBuilder.cpp
        wallet/common/CurrencyBuilder.hpp wallet/pool/database/CurrenciesDatabaseHelper.cpp
        wallet/pool/database/CurrenciesDatabaseHelper.hpp wallet/pool/database/PoolDatabaseHelper.cpp
        wallet/pool/database/PoolDatabaseHelper.hpp wallet/common/AbstractWalletFactory.cpp
        wallet/common/AbstractWalletFactory.hpp wallet/pool/database/WalletDatabaseEntry.cpp
        wallet/pool/database/WalletDatabaseEntry.hpp utils/ConfigurationMatchable.cpp utils/ConfigurationMatchable.h
        wallet/bitcoin/synchronizers/BlockchainExplorerAccountSynchronizer.cpp
        wallet/bitcoin/synchronizers/BlockchainExplorerAccountSynchronizer.h
        events/ProgressNotifier.cpp events/ProgressNotifier.h
        wallet/bitcoin/database/BitcoinLikeWalletDatabase.cpp wallet/bitcoin/database/BitcoinLikeWalletDatabase.h
        wallet/common/database/AccountDatabaseHelper.cpp wallet/common/database/AccountDatabaseHelper.h
        wallet/bitcoin/database/BitcoinLikeAccountDatabaseEntry.cpp
        wallet/bitcoin/database/BitcoinLikeAccountDatabaseEntry.h
        wallet/bitcoin/database/BitcoinLikeAccountDatabase.cpp
        wallet/bitcoin/database/BitcoinLikeAccountDatabase.h
        wallet/common/database/OperationDatabaseHelper.cpp
        wallet/common/database/OperationDatabaseHelper.h
        utils/JSONUtils.h database/soci-option.h database/soci-date.h
        wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.cpp
        wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h database/soci-number.h
        wallet/common/Operation.cpp wallet/common/Operation.h wallet/common/TrustIndicator.cpp
        wallet/common/TrustIndicator.h wallet/bitcoin/factories/BitcoinLikeKeychainFactory.cpp
        wallet/bitcoin/factories/BitcoinLikeKeychainFactory.h
        wallet/bitcoin/factories/keystores/BitcoinLikeP2PKHKeychainFactory.cpp
        wallet/bitcoin/factories/keystores/BitcoinLikeP2PKHKeychainFactory.h
        wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.cpp
        wallet/bitcoin/database/BitcoinLikeAccountDatabaseHelper.h
        database/query/QueryBuilder.cpp database/query/QueryBuilder.h database/query/QueryFilter.cpp
        database/query/QueryFilter.h database/query/CompoundQueryFilter.cpp database/query/CompoundQueryFilter.h
        database/query/filters_impl.cpp database/query/ConditionQueryFilter.cpp database/query/ConditionQueryFilter.h
        wallet/common/Amount.cpp wallet/common/Amount.h wallet/common/OperationQuery.cpp wallet/common/OperationQuery.h
        wallet/common/api_impl/OperationApi.cpp wallet/common/api_impl/OperationApi.h
        wallet/common/database/BlockDatabaseHelper.cpp wallet/common/database/BlockDatabaseHelper.h
        wallet/common/Block.cpp wallet/common/Block.h
        debug/LoggerStreamBuffer.cpp debug/LoggerStreamBuffer.h
        wallet/bitcoin/api_impl/BitcoinLikeOperation.cpp wallet/bitcoin/api_impl/BitcoinLikeOperation.h
        wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.cpp wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h
        wallet/bitcoin/api_impl/BitcoinLikeInputApi.cpp wallet/bitcoin/api_impl/BitcoinLikeInputApi.h
        wallet/bitcoin/api_impl/BitcoinLikeOutputApi.cpp wallet/bitcoin/api_impl/BitcoinLikeOutputApi.h
        wallet/bitcoin/api_impl/BitcoinLikeBlockApi.cpp wallet/bitcoin/api_impl/BitcoinLikeBlockApi.h async/wait.h
        async/algorithm.h debug/Benchmarker.cpp
        debug/Benchmarker.h utils/DurationUtils.cpp
        utils/DurationUtils.h utils/VectorUtils.cpp utils/VectorUtils.h
        crypto/Secp256k1Api.cpp crypto/Secp256k1Api.h
        wallet/bitcoin/database/BitcoinLikeUTXODatabaseHelper.cpp wallet/bitcoin/database/BitcoinLikeUTXODatabaseHelper.h
        wallet/bitcoin/database/BitcoinLikeBlockDatabaseHelper.cpp wallet/bitcoin/database/BitcoinLikeBlockDatabaseHelper.h
        wallet/bitcoin/observers/LedgerApiBitcoinLikeBlockchainObserver.cpp wallet/bitcoin/observers/LedgerApiBitcoinLikeBlockchainObserver.h
        wallet/bitcoin/observers/BitcoinLikeBlockchainObserver.cpp
        net/WebSocketClient.cpp net/WebSocketClient.h
        net/WebSocketConnection.cpp net/WebSocketConnection.h
        math/Fibonacci.cpp math/Fibonacci.h
        wallet/bitcoin/explorers/api/WebSocketNotificationParser.cpp wallet/bitcoin/explorers/api/WebSocketNotificationParser.h wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.cpp wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h bitcoin/BitcoinLikeHelper.cpp bitcoin/BitcoinLikeHelper.h bitcoin/ScriptParser.cpp bitcoin/ScriptParser.h bitcoin/opcodes.h bitcoin/opcodes.cpp)

target_link_libraries(ledger-core bigd)
target_link_libraries(ledger-core fmt)
target_link_libraries(ledger-core crypto)
target_link_libraries(ledger-core sqlite3)
target_link_libraries(ledger-core soci_sqlite3)
target_link_libraries(ledger-core soci_core_static)
target_link_libraries(ledger-core leveldb)
target_link_libraries(ledger-core "${CMAKE_SOURCE_DIR}/core/lib/secp256k1/secp256k1/lib/libsecp256k1.a")

link_directories("${CMAKE_SOURCE_DIR}/core/lib/secp256k1/secp256k1/lib")
file(GLOB_RECURSE LEDGER_CORE_HEADERS
        "*.hpp"
        "*.h"
        )

file(GLOB LEDGER_CORE_API_HEADERS
        "api/*.hpp"
        )

include_directories(ledger-core ../lib/bigd)

if (TARGET_JNI)
    include_directories(ledger-core ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core ${JNI_LIBRARIES})
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
target_include_directories(ledger-core PUBLIC ../lib/date/)
target_include_directories(ledger-core PUBLIC ../lib/secp256k1/secp256k1)

install(TARGETS ledger-core DESTINATION "lib")
install(FILES ${LEDGER_CORE_API_HEADERS} DESTINATION "include/ledger-core")
