/*
 *
 * cosmos_synchronization_benchmarks.cpp
 * Benchmarks for cosmos_synchronization
 * Usage :
 * ledger-core-integration-tests --gtest_filter=CosmosWalletSyncBenchmark.Medium
 * Medium is the size of the account in number of operations.
 * Currently Small, Medium, Large, ExtraLarge and Huge are supported
 * ledger-core
 *
 * Created by Gerry Agbobada on 13/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "../BaseFixture.h"

#include <chrono>
#include <iostream>
#include <set>

#include <api/Configuration.hpp>
#include <api/CosmosConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <collections/DynamicObject.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <gtest/gtest.h>
#include <math/BigInt.h>
#include <test/cosmos/Fixtures.hpp>
#include <utils/DateUtils.hpp>
#include <utils/hex.h>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>
#include <wallet/cosmos/explorers/GaiaCosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>

using namespace std;
using namespace ledger::core;
using namespace ledger::testing::cosmos;

namespace {
api::CosmosLikeNetworkParameters PARAMS = networks::getCosmosLikeNetworkParameters("atom");
const std::string HUGE_PUBKEY =
    "03D13FD82D9389BBBEA20BAE392D80C0EF6C1B0CB57A37ACFAEF2105D189DA0BA2";
const std::string HUGE_ADDRESS = "cosmos1y6yvdel7zys8x60gz9067fjpcpygsn62ae9x46";
const std::string EXTRA_LARGE_PUBKEY =
    "037587CAF2B1724392DAC68EC019B27F02743387B8015830BB7C5FD63472DAC32D";
const std::string EXTRA_LARGE_ADDRESS = "cosmos15v50ymp6n5dn73erkqtmq0u8adpl8d3ujv2e74";
const std::string LARGE_PUBKEY =
    "038665FEF8CCDC37B367E90ECE899C8689FA4225FC38F2EE3F60A637F9DD96C47A";
const std::string LARGE_ADDRESS = "cosmos1nynns8ex9fq6sjjfj8k79ymkdz4sqth06xexae";
const std::string MEDIUM_PUBKEY =
    "03175B1A69FC4A7C4047A12BC2BD32F144F29CA6C1DE65A2486DEDE55EFA472E4B";
const std::string MEDIUM_ADDRESS = "cosmos156gqf9837u7d4c4678yt3rl4ls9c5vuuxyhkw6";
const std::string SMALL_PUBKEY =
    "03d672c1b90c84d9d97522e9a73252a432b77d90a78bf81cdbe35270d9d3dc1c34";
const std::string SMALL_ADDRESS = "cosmos1sd4tl9aljmmezzudugs7zlaya7pg2895tyn79r";
}  // namespace

class CosmosWalletSyncBenchmark : public BaseFixture {
   public:
    void SetUp() override
    {
        BaseFixture::SetUp();
        auto worker = dispatcher->getSerialExecutionContext("worker");
        auto threadpoolWorker = dispatcher->getThreadPoolExecutionContext("threadpoolWorker");
        auto client = std::make_shared<HttpClient>(
            api::CosmosConfigurationDefaults::COSMOS_DEFAULT_API_ENDPOINT, http, worker, threadpoolWorker);

#ifdef PG_SUPPORT
        const bool usePostgreSQL = true;
        auto poolConfig = DynamicObject::newInstance();
        poolConfig->putString(
            api::PoolConfiguration::DATABASE_NAME, "postgres://localhost:5432/test_db");
        pool = newDefaultPool("postgres", "", poolConfig, usePostgreSQL);
#else
        pool = newDefaultPool();
#endif

        explorer = std::make_shared<GaiaCosmosLikeBlockchainExplorer>(
            worker, client, PARAMS, std::make_shared<DynamicObject>());
    }

    void setupTest(
        std::shared_ptr<CosmosLikeAccount> &account,
        std::shared_ptr<AbstractWallet> &wallet,
        const std::string &pubKey)
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(
            api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
            "44'/<coin_type>'/<account>'/<node>/<address>");
        wallet = uv::wait(
            pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "cosmos", configuration));

        auto accountInfo = uv::wait(wallet->getNextAccountCreationInfo());
        EXPECT_EQ(accountInfo.index, 0);
        accountInfo.publicKeys.push_back(hex::toByteArray(pubKey));

        account = ledger::testing::cosmos::createCosmosLikeAccount(
            wallet, accountInfo.index, accountInfo);
    }

    void performSynchro(const std::shared_ptr<CosmosLikeAccount> &account)
    {
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));
            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
                return;
            }
            EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

            auto balance = uv::wait(account->getBalance());
            fmt::print("Balance: {} uatom\n", balance->toString());

            auto block = uv::wait(account->getLastBlock());
            fmt::print("Block height: {}\n", block.height);
            EXPECT_GT(block.height, 0);

            getTestExecutionContext()->stop();
        });

        auto bus = account->synchronize();
        bus->subscribe(getTestExecutionContext(), receiver);
        getTestExecutionContext()->waitUntilStopped();
    }

    void TearDown() override
    {
        uv::wait(pool->freshResetAll());
        BaseFixture::TearDown();
    }

    std::shared_ptr<WalletPool> pool;
    std::shared_ptr<GaiaCosmosLikeBlockchainExplorer> explorer;
};

TEST_F(CosmosWalletSyncBenchmark, DISABLED_Small)
{
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, SMALL_PUBKEY);

    auto start = std::chrono::system_clock::now();
    performSynchro(account);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to synchronize " << SMALL_ADDRESS << " : " << diff.count() << " s\n";

    start = std::chrono::system_clock::now();
    auto ops =
        uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())
                 ->execute());
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Time to query its " << ops.size() << " operations : " << diff.count() << " s\n";
}

TEST_F(CosmosWalletSyncBenchmark, DISABLED_Medium)
{
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, MEDIUM_PUBKEY);

    auto start = std::chrono::system_clock::now();
    performSynchro(account);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to synchronize " << MEDIUM_ADDRESS << " : " << diff.count() << " s\n";

    start = std::chrono::system_clock::now();
    auto ops =
        uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())
                 ->execute());
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Time to query its " << ops.size() << " operations : " << diff.count() << " s\n";
}

TEST_F(CosmosWalletSyncBenchmark, DISABLED_Large)
{
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, LARGE_PUBKEY);

    auto start = std::chrono::system_clock::now();
    performSynchro(account);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to synchronize " << LARGE_ADDRESS << " : " << diff.count() << " s\n";

    start = std::chrono::system_clock::now();
    auto ops =
        uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())
                 ->execute());
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Time to query its " << ops.size() << " operations : " << diff.count() << " s\n";
}

TEST_F(CosmosWalletSyncBenchmark, DISABLED_ExtraLarge)
{
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, EXTRA_LARGE_PUBKEY);

    auto start = std::chrono::system_clock::now();
    performSynchro(account);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to synchronize " << EXTRA_LARGE_ADDRESS << " : " << diff.count() << " s\n";

    start = std::chrono::system_clock::now();
    auto ops =
        uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())
                 ->execute());
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Time to query its " << ops.size() << " operations : " << diff.count() << " s\n";
}

TEST_F(CosmosWalletSyncBenchmark, DISABLED_Huge)
{
    std::shared_ptr<CosmosLikeAccount> account;
    std::shared_ptr<AbstractWallet> wallet;

    setupTest(account, wallet, HUGE_PUBKEY);

    auto start = std::chrono::system_clock::now();
    performSynchro(account);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to synchronize " << HUGE_ADDRESS << " : " << diff.count() << " s\n";

    start = std::chrono::system_clock::now();
    auto ops =
        uv::wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())
                 ->execute());
    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Time to query its " << ops.size() << " operations : " << diff.count() << " s\n";
}
