cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ANDROID_CPP_FEATURES exceptions)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(ledger-core 0.1.0 REQUIRED)

# Automatically add API files to the library
file(
    GLOB ledger-core-bitcoin-api-sources
    "${CMAKE_SOURCE_DIR}/inc/bitcoin/api/*"
)
list(APPEND ledger-core-bitcoin-sources ${ledger-core-bitcoin-api-sources})

# Configure JNI target
if (TARGET_JNI)
    message(STATUS "Configure project for JNI")

    file(
        GLOB ledger-core-bitcoin-jni-sources
        "jni/*.cpp"
        "jni/*.hpp"
        "jni/jni/*"
    )

    if(NOT ANDROID)
        find_package(JNI REQUIRED)
    endif()

    list(APPEND ledger-core-bitcoin-sources ${ledger-core-bitcoin-jni-sources})
    add_definitions(-DTARGET_JNI=1)
endif ()

# Add files to compile to the project
set(SRC_FILES
    # bech32
    bitcoin/bech32/BCHBech32.cpp
    bitcoin/bech32/Bech32.cpp
    bitcoin/bech32/Bech32Factory.cpp
    bitcoin/bech32/Bech32Parameters.cpp
    bitcoin/bech32/BTCBech32.cpp
    # block
    bitcoin/block/BitcoinLikeBlock.cpp
    # database
    bitcoin/database/BitcoinLikeAccountDatabase.cpp
    bitcoin/database/BitcoinLikeAccountDatabaseHelper.cpp
    bitcoin/database/BitcoinLikeBlockDatabaseHelper.cpp
    bitcoin/database/BitcoinLikeTransactionDatabaseHelper.cpp
    bitcoin/database/BitcoinLikeUTXODatabaseHelper.cpp
    bitcoin/database/BitcoinLikeWalletDatabase.cpp
    bitcoin/database/Migrations.cpp
    # explorers
    bitcoin/explorers/BitcoinLikeBlockchainExplorer.cpp
    bitcoin/explorers/LedgerApiBitcoinLikeBlockchainExplorer.cpp
    # factories
    bitcoin/factories/BitcoinLikeWalletFactory.cpp
    # io
    bitcoin/io/BitcoinLikeInput.cpp
    bitcoin/io/BitcoinLikeInputParser.cpp
    bitcoin/io/BitcoinLikeOutput.cpp
    bitcoin/io/BitcoinLikeOutputParser.cpp
    bitcoin/io/BitcoinLikeWritableInput.cpp
    # keychains
    bitcoin/keychains/BitcoinLikeKeychain.cpp
    bitcoin/keychains/CommonBitcoinLikeKeychains.cpp
    bitcoin/keychains/P2PKHBitcoinLikeKeychain.cpp
    bitcoin/keychains/P2SHBitcoinLikeKeychain.cpp
    bitcoin/keychains/P2WPKHBitcoinLikeKeychain.cpp
    bitcoin/keychains/P2WSHBitcoinLikeKeychain.cpp
    # observers
    bitcoin/observers/BitcoinLikeBlockchainObserver.cpp
    bitcoin/observers/LedgerApiBitcoinLikeBlockchainObserver.cpp
    # operations
    bitcoin/operations/BitcoinLikeOperation.cpp
    bitcoin/operations/BitcoinLikeOperationQuery.cpp
    # scripts
    bitcoin/scripts/BitcoinLikeScriptOperators.cpp
    bitcoin/scripts/BitcoinLikeScript.cpp
    bitcoin/scripts/BitcoinLikeInternalScript.cpp
    # synchronizers
    bitcoin/synchronizers/BitcoinLikeBlockchainExplorerAccountSynchronizer.cpp
    # transactions
    bitcoin/transactions/BitcoinLikeStrategyUTXOPicker.cpp
    bitcoin/transactions/BitcoinLikeTransaction.cpp
    bitcoin/transactions/BitcoinLikeTransactionBuilder.cpp
    bitcoin/transactions/BitcoinLikeTransactionParser.cpp
    bitcoin/transactions/BitcoinLikeUTXOPicker.cpp
    # others
    bitcoin/BitcoinLikeAccount.cpp
    bitcoin/BitcoinLikeAddress.cpp
    bitcoin/BitcoinLikeCurrencies.cpp
    bitcoin/BitcoinLikeExtendedPublicKey.cpp
    bitcoin/BitcoinLikeWallet.cpp
    bitcoin/BitcoinNetworks.cpp
)

# Interface library used to hold shared properties
add_library(ledger-core-bitcoin-interface INTERFACE)

target_compile_definitions(ledger-core-bitcoin-interface INTERFACE ledger_core_EXPORTS)
target_compile_definitions(ledger-core-bitcoin-interface INTERFACE
    LIB_VERSION_MAJOR=${VERSION_MAJOR}
    LIB_VERSION_MINOR=${VERSION_MINOR}
    LIB_VERSION_PATCH=${VERSION_PATCH}
)

string(FIND "${CMAKE_OSX_SYSROOT}" "iphone" IS_IOS)
if(IS_IOS GREATER_EQUAL 0)
    add_library(ledger-core-bitcoin SHARED
        ${ledger-core-bitcoin-sources}
        ${SRC_FILES}
        ${CORE_SRC_FILES}
        ${HEADERS_FILES})
    target_link_libraries(ledger-core-bitcoin PUBLIC ledger-core-bitcoin-interface)
    set(CMAKE_SHARED_LINKER_FLAGS "-Wall")
    set(FRAMEWORK_BUNDLE_IDENTIFIER "com.ledger.core")
    set(DEPLOYMENT_TARGET 9.0)
    set(DEVICE_FAMILY "1")
    set(PRODUCT_NAME ledger_core)
    set_target_properties(ledger-core-bitcoin PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER ${FRAMEWORK_BUNDLE_IDENTIFIER}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION_MAJOR}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
        MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_BINARY_DIR}/framework.plist.in
        # "current version" in semantic format in Mach-O binary file
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        # "compatibility version" in semantic format in Mach-O binary file
        SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        PUBLIC_HEADER "${CMAKE_BINARY_DIR}/include/ledger/core"
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET}
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY ${DEVICE_FAMILY}
        XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
    )
    add_custom_command(
        TARGET ledger-core-bitcoin
        POST_BUILD
        COMMAND /bin/bash -c "${CMAKE_BINARY_DIR}/install_name.sh \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME} ${CMAKE_OSX_ARCHITECTURES}"
    )
    add_custom_command(
        TARGET ledger-core-bitcoin
        POST_BUILD
        COMMAND install_name_tool -id \"@rpath/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}\"
        \${BUILT_PRODUCTS_DIR}/\${PRODUCT_NAME}.framework/\${PRODUCT_NAME}
    )
else()
    add_library(ledger-core-bitcoin-obj OBJECT
        ${ledger-core-bitcoin-sources}
        ${SRC_FILES}
        ${CORE_SRC_FILES}
        ${HEADERS_FILES}
        ${CORE_HEADER_FILES})
    target_link_libraries(ledger-core-bitcoin-obj PUBLIC ledger-core-bitcoin-interface)

    # shared and static libraries built from the same object files
    add_library(ledger-core-bitcoin SHARED)
    target_link_libraries(ledger-core-bitcoin PUBLIC ledger-core-bitcoin-obj)

    add_library(ledger-core-bitcoin-static STATIC)
    target_link_libraries(ledger-core-bitcoin-static PUBLIC ledger-core-bitcoin-obj)
    install(TARGETS ledger-core-bitcoin-static DESTINATION "lib")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(ledger-core-bitcoin-interface INTERFACE -static-libstdc++)
endif()

# link the ledger-core library
target_link_libraries(ledger-core-bitcoin-interface INTERFACE Core::ledger-core)

if (TARGET_JNI)
    target_include_directories(ledger-core-bitcoin-interface INTERFACE ${JNI_INCLUDE_DIRS})
    target_link_libraries(ledger-core-bitcoin-interface INTERFACE ${JNI_LIBRARIES})
endif ()

target_include_directories(ledger-core-bitcoin-interface INTERFACE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(ledger-core-bitcoin-interface INTERFACE ${CMAKE_SOURCE_DIR}/inc/bitcoin/api)

install(TARGETS ledger-core-bitcoin DESTINATION lib)
