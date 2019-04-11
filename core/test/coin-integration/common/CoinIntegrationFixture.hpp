/*
 *
 * CoinIntegrationFixture.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef LEDGER_CORE_COININTEGRATIONFIXTURE_HPP
#define LEDGER_CORE_COININTEGRATIONFIXTURE_HPP

#include <gtest/gtest.h>
#include <async/QtThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <async/async_wait.h>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BigInt.hpp>
#include <net/QtHttpClient.hpp>
#include <events/LambdaEventReceiver.hpp>
#include <soci.h>
#include <api/Account.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <FakeWebSocketClient.h>
#include <OpenSSLRandomNumberGenerator.hpp>
#include <utils/FilesystemUtils.h>
#include <utils/hex.h>
#include "../integration/IntegrationEnvironment.h"

using namespace ledger::core; // don't do this at home. Only for testing contexts
using namespace ledger::qt;

enum SynchronizationResult {OLD_ACCOUNT, NEW_ACCOUNT};

template <typename Wallet, typename Account>
class CoinIntegrationFixture : public ::testing::Test {
public:
    void SetUp() override {
        ::testing::Test::SetUp();
        ledger::qt::FilesystemUtils::clearFs(IntegrationEnvironment::getInstance()->getApplicationDirPath());
        dispatcher = std::make_shared<QtThreadDispatcher>();
        resolver = std::make_shared<NativePathResolver>(IntegrationEnvironment::getInstance()->getApplicationDirPath());
        backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
        printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
        http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
        ws = std::make_shared<FakeWebSocketClient>();
        rng = std::make_shared<OpenSSLRandomNumberGenerator>();
    }

    void TearDown() override {
        ::testing::Test::TearDown();
        resolver->clean();
    }

    virtual std::shared_ptr<WalletPool> newPool(std::string poolName = "my_pool") {
        return WalletPool::newInstance(
                poolName,
                Option<std::string>(),
                http,
                ws,
                resolver,
                printer,
                dispatcher,
                rng,
                backend,
                api::DynamicObject::newInstance()
        );
    }

    void injectCurrency(const std::shared_ptr<WalletPool>& pool, const api::Currency& currency) {
        ::wait(pool->addCurrency(currency));
    }

    std::shared_ptr<Wallet> newWallet(  const std::shared_ptr<WalletPool>& pool,
                                        const std::string& walletName,
                                        const std::string& currencyName,
                                        const std::shared_ptr<api::DynamicObject> &configuration
                                     ) {
        return std::dynamic_pointer_cast<Wallet>(::wait(pool->createWallet(walletName, currencyName, configuration)));
    }

    std::shared_ptr<Account> newAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::AccountCreationInfo &info) {
        auto i = info;
        i.index = index;
        return std::dynamic_pointer_cast<Account>(::wait(wallet->newAccountWithInfo(i)));
    }

    std::shared_ptr<Account> newAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::ExtendedKeyAccountCreationInfo& info) {
        auto i = info;
        i.index = index;
        return std::dynamic_pointer_cast<Account>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
    }


    Try<SynchronizationResult> synchronizeAccount(const std::shared_ptr<AbstractAccount>& account) {
        Promise<SynchronizationResult> p;
        auto bus = account->synchronize();
        bus->subscribe(dispatcher->getSerialExecutionContext("callback"),
                       make_receiver([=](const std::shared_ptr<api::Event> &event) mutable {
                           if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
                               p.success(SynchronizationResult::NEW_ACCOUNT);
                           } else if (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED) {
                               p.success(SynchronizationResult::OLD_ACCOUNT);
                           } else if (event->getCode() == api::EventCode::SYNCHRONIZATION_FAILED) {
                               api::ErrorCode code = api::ErrorCode::UNKNOWN;
                               std::string reason = "Unknown error";
                               auto payload = event->getPayload();
                               if (payload && payload->getString(api::Account::EV_SYNC_ERROR_CODE)) {
                                   code = api::from_string<api::ErrorCode>(payload->getString(api::Account::EV_SYNC_ERROR_CODE).value());
                               }
                               if (payload && payload->getString(api::Account::EV_SYNC_ERROR_MESSAGE)) {
                                   reason = payload->getString(api::Account::EV_SYNC_ERROR_MESSAGE).value();
                               }
                               p.failure(make_exception(code, reason));
                           }
                       }));
        return ::wait(p.getFuture());
    }

    std::shared_ptr<QtThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<QtHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;

};

#endif //LEDGER_CORE_COININTEGRATIONFIXTURE_HPP
