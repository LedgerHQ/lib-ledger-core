/*
 *
 * BaseFixture.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once

#include <gtest/gtest.h>
#include <soci.h>
#include <unordered_set>

#include <async/AsyncWait.hpp>
#include <async/QtThreadDispatcher.hpp>
#include <net/QtHttpClient.hpp>
#include <CoutLogPrinter.hpp>
#include <FakeWebSocketClient.hpp>
#include <NativePathResolver.hpp>
#include <OpenSSLRandomNumberGenerator.hpp>

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <core/api/BigInt.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/utils/JSONUtils.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>
#include <core/wallet/CurrencyBuilder.hpp>
#include <core/wallet/WalletDatabaseHelper.hpp>

using namespace ledger::core; // Only do that for testing
using namespace ledger::qt; // Djeez

extern api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo P2WPKH_MEDIUM_XPUB_INFO;
extern api::AccountCreationInfo P2PKH_MEDIUM_KEYS_INFO;
extern api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo P2SH_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_ROPSTEN_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_MAIN_XPUB_INFO;
extern api::AccountCreationInfo ETH_KEYS_INFO;
extern api::AccountCreationInfo ETH_KEYS_INFO_VAULT;
extern api::AccountCreationInfo ETH_KEYS_INFO_LIVE;
extern api::AccountCreationInfo ETC_KEYS_INFO_LIVE;
extern api::AccountCreationInfo XTZ_KEYS_INFO;
extern const std::string TX_1;
extern const std::string TX_2;
extern const std::string TX_3;
extern const std::string TX_4;

class BaseFixture : public ::testing::Test {
public:
    virtual ~BaseFixture() = default;

    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<Services> newDefaultServices(
        const std::string &tenant = "default_tenant",
        const std::string &password = "test"
    );

    void createWallet(
        const std::shared_ptr<Services>& services,
        const std::string& walletName,
        const std::string& currencyName,
        const std::shared_ptr<api::DynamicObject> &configuration
    );

    void createAccount(
        const std::shared_ptr<Services>& services,
        const std::string &walletName,
        int32_t index
    );

    std::shared_ptr<QtThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<QtHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;
};
