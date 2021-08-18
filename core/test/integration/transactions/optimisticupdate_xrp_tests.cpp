/*
 *
 * optimisticupdate_xrp_tests.cpp
 * ledger-core
 *
 * Created by Léo Lehmann on 10/07/2020.
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/ConfigurationDefaults.hpp>
#include <api/KeychainEngines.hpp>
#include <api/RippleLikeTransaction.hpp>
#include <api/OperationOrderKey.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <wallet/ripple/api_impl/RippleLikeTransactionApi.h>
#include <api/RippleConfigurationDefaults.hpp>
#include <iostream>
#include "FakeHttpClient.hpp"

using namespace std;

class RippleLikeOptimisticTransactionUpdate : public BaseFixture
{
};

std::pair<std::shared_ptr<LambdaEventReceiver>, ledger::core::Future<bool>> createSyncReceiver();

TEST_F(RippleLikeOptimisticTransactionUpdate, BroadcastTransaction)
{

    auto fakeHttp = std::make_shared<test::FakeHttpClient>();

    backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());

    auto pool = WalletPool::newInstance(
        "my_pool",
        "",
        fakeHttp,
        ws,
        resolver,
        printer,
        dispatcher,
        rng,
        backend,
        api::DynamicObject::newInstance(),
        nullptr,
        nullptr);
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, "http://test.test");
        auto wallet = uv::wait(pool->createWallet(randomWalletName(), "ripple", configuration));
        {
            auto account = createRippleLikeAccount(wallet, 0, XRP_KEYS_INFO);
            auto waiter = createSyncReceiver();

            fakeHttp->setBehavior({{fmt::format("http://test.test:{}/", api::RippleConfigurationDefaults::RIPPLE_DEFAULT_PORT),
                                    test::FakeUrlConnection::fromString("{\"result\":{\"engine_result\":\"tesSUCCESS\",\"tx_json\":{\"hash\":\"AC0D84CB81E8ECA92E7EF9ABC3526FAED54DE07763A308296B28468D68D34991\"}}}")}});
            
            auto dummy_transaction = make_shared<RippleLikeTransactionApi>(account->getWallet()->getCurrency());
            dummy_transaction->setSequence(BigInt(1));
            dummy_transaction->setLedgerSequence(BigInt(1));
            dummy_transaction->setValue(make_shared<BigInt>(1));
            dummy_transaction->setFees(make_shared<BigInt>(1));
            dummy_transaction->setSender(account->getKeychain()->getAddress());
            dummy_transaction->setReceiver(account->getKeychain()->getAddress());
            dummy_transaction->setHash("AC0D84CB81E8ECA92E7EF9ABC3526FAED54DE07763A308296B28468D68D34991");
            std::vector<uint8_t> dummy_key = {1 , 2 , 3 , 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
            dummy_transaction->setSigningPubKey(dummy_key);
            dummy_transaction->setDERSignature(dummy_key);

            auto tx_hash = uv::wait(account->broadcastTransaction(dynamic_pointer_cast<api::RippleLikeTransaction>(dummy_transaction)));
            auto explorer_tx = uv::wait(account->getTransaction(tx_hash));

            EXPECT_EQ(explorer_tx->hash, "AC0D84CB81E8ECA92E7EF9ABC3526FAED54DE07763A308296B28468D68D34991");
            EXPECT_EQ(explorer_tx->value, BigInt(1));
            EXPECT_EQ(explorer_tx->fees, BigInt(1));
            EXPECT_EQ(explorer_tx->sequence, BigInt(1));
            EXPECT_EQ(explorer_tx->receiver, account->getKeychain()->getAddress()->toString());
            EXPECT_EQ(explorer_tx->sender, account->getKeychain()->getAddress()->toString());
            EXPECT_EQ(explorer_tx->confirmations, 0);
            EXPECT_EQ(explorer_tx->status, 1);
        }
    }
}
