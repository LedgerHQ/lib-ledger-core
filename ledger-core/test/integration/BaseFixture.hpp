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

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/AccountCreationInfo.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/events/LambdaEventReceiver.hpp>
#include <core/wallet/WalletStore.hpp>

#include <async/AsyncWait.hpp>
#include <async/QtThreadDispatcher.hpp>
#include <net/QtHttpClient.hpp>
#include <CoutLogPrinter.hpp>
#include <FakeWebSocketClient.hpp>
#include <NativePathResolver.hpp>
#include <OpenSSLRandomNumberGenerator.hpp>

using namespace ledger::core; // Only do that for testing
using namespace ledger::qt; // Djeez

class BaseFixture : public ::testing::Test {
public:
    virtual ~BaseFixture() = default;

    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<Services> newDefaultServices(
        const std::string &tenant = "default_tenant",
        const std::string &password = "test",
        const std::shared_ptr<api::DynamicObject> &configuration = api::DynamicObject::newInstance()
    );

    std::shared_ptr<WalletStore> newWalletStore(
        const std::shared_ptr<Services>& services
    );

    void createWalletInDatabase(
        const std::shared_ptr<Services>& services,
        const std::string& walletName,
        const std::string& currencyName,
        const std::shared_ptr<api::DynamicObject> &configuration
    );

    void createAccountInDatabase(
        const std::shared_ptr<Services>& services,
        const std::string &walletName,
        int32_t index
    );

    template<typename Account>
    std::shared_ptr<Account> createAccount(
        std::shared_ptr<AbstractWallet> const &wallet,
        int32_t index,
        api::AccountCreationInfo info) 
    {
        info.index = index;

        return std::dynamic_pointer_cast<Account>(wait(wallet->newAccountWithInfo(info)));
    }

    template <typename Account>
    std::shared_ptr<Account> createAccount(
        std::shared_ptr<AbstractWallet> const &wallet, 
        int32_t index,
        api::ExtendedKeyAccountCreationInfo info) 
    {
        info.index = index;
        
        return std::dynamic_pointer_cast<Account>(wait(wallet->newAccountWithExtendedKeyInfo(info)));
    } 

    std::shared_ptr<QtThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<QtHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;
};
