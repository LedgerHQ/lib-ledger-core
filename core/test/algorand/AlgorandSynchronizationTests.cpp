/*
 * AlgorandSynchronizationTests
 *
 * Created by Hakim Aammar on 01/06/2020.
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

#include "AlgorandTestFixtures.hpp"
#include <wallet/algorand/AlgorandAccount.hpp>
#include <wallet/algorand/AlgorandWalletFactory.hpp>
#include <wallet/algorand/AlgorandWallet.hpp>
#include <wallet/algorand/AlgorandLikeCurrencies.hpp>
#include <wallet/algorand/AlgorandNetworks.hpp>
#include <wallet/common/OperationQuery.h>
#include <api/Configuration.hpp>

#include "../integration/WalletFixture.hpp"

#include <functional>

using namespace ledger::testing::algorand;
using namespace ledger::core::algorand;

class AlgorandSynchronizationTest : public WalletFixture<WalletFactory> {
public:
    void SetUp() override {
        WalletFixture::SetUp();
        backend->enableQueryLogging(true);
    }
    void TearDown() override {
        uv::wait(pool->deleteWallet("test-wallet"));
        WalletFixture::TearDown();
    }
    void synchronizeAccount(const std::string & accountAddress) {
        registerCurrency(currencies::ALGORAND);

        // NOTE: we run the tests on the staging environment which is on the TestNet
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "https://algorand.coin.staging.aws.ledger.com");

        auto wallet = std::dynamic_pointer_cast<Wallet>(uv::wait(pool->createWallet("test-wallet", "algorand", configuration)));

        auto nextIndex = uv::wait(wallet->getNextAccountIndex());
        EXPECT_EQ(nextIndex, 0);

        const api::AccountCreationInfo info(
            nextIndex,
            {"main"},
            {"44'/283'/0'/0'"},
            { algorand::Address::toPublicKey(accountAddress) },
            {hex::toByteArray("")}
        );

        _account = std::dynamic_pointer_cast<Account>(uv::wait(wallet->newAccountWithInfo(info)));

        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
            fmt::print("Received event {}\n", api::to_string(event->getCode()));

            if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) {
                return;
            }
            EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);

            getTestExecutionContext()->stop();
        });

        auto bus = _account->synchronize();
        bus->subscribe(getTestExecutionContext(), receiver);
        getTestExecutionContext()->waitUntilStopped();
    }

    std::shared_ptr<Account> _account;

};

TEST_F(AlgorandSynchronizationTest, DISABLED_EmptyAccountSynchronizationTest) {
    synchronizeAccount(EMPTY_ADDRESS);

    // Simulate the getLastBlock that Live does after an account sync
    // Just check nothing goes wrong
    EXPECT_NO_THROW(uv::wait(pool->getLastBlock(currencies::ALGORAND.name)));

    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(_account->queryOperations()->complete())->execute());
    EXPECT_EQ(operations.size(), 0);

}

TEST_F(AlgorandSynchronizationTest, DISABLED_AccountSynchronizationTest) {
    synchronizeAccount(OBELIX_ADDRESS);

    auto internalPreferences = _account->getInternalPreferences()->getSubPreferences("AlgorandAccountSynchronizer");
    auto savedState = internalPreferences->template getObject<SavedState>("state");
    std::cout << ">>> Saved block round for next synchronization: " << savedState.getValue().round << std::endl;
    EXPECT_GT(savedState.getValue().round, 6000000);

    auto operations = uv::wait(std::dynamic_pointer_cast<OperationQuery>(_account->queryOperations()->complete())->execute());
    std::cout << ">>> Nb of operations: " << operations.size() << std::endl;
    EXPECT_GT(operations.size(), 200);
}
