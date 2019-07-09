/*
 *
 * ripple_synchronization
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#include <gtest/gtest.h>
#include "../BaseFixture.h"
#include <set>
#include <api/KeychainEngines.hpp>
#include <utils/DateUtils.hpp>
#include <wallet/ripple/database/RippleLikeAccountDatabaseHelper.h>
#include <wallet/ripple/transaction_builders/RippleLikeTransactionBuilder.h>
#include <iostream>
#include <api/BlockchainExplorerEngines.hpp>
using namespace std;

class RippleLikeWalletSynchronization : public BaseFixture {

};

TEST_F(RippleLikeWalletSynchronization, MediumXpubSynchronization) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration));
        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto account = createRippleLikeAccount(wallet, nextIndex, XRP_KEYS_INFO);

            auto fees = wait(account->getFees());
            EXPECT_GT(fees->toLong(), 0L);
            auto baseReserve = wait(account->getBaseReserve());
            EXPECT_GT(baseReserve->toLong(), 0L);

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            receiver.reset();
            receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                auto balance = wait(account->getBalance());
                std::cout << "Balance: " << balance->toString() << std::endl;
                auto txBuilder = std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            dispatcher->waitUntilStopped();

            auto ops = wait(
                    std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
            std::cout << "Ops: " << ops.size() << std::endl;

            auto block = wait(account->getLastBlock());
            auto blockHash = block.blockHash;

        }
    }
}


const std::string NOTIF_WITH_TX = "{\"engine_result\":\"tecDST_TAG_NEEDED\",\"engine_result_code\":143,\"engine_result_message\":\"A destination tag is required.\",\"ledger_hash\":\"6C048E47F2478A5F5FD936B93F8C16AB3D7EC92EF7736AACD9BFB363F0F2FF4A\",\"ledger_index\":44352210,\"meta\":{\"AffectedNodes\":[{\"ModifiedNode\":{\"FinalFields\":{\"Account\":\"r9Jsby859fEyc5tG4z5saATiYEhSLg1Cgu\",\"Balance\":\"1281979746\",\"Flags\":0,\"OwnerCount\":0,\"Sequence\":969193},\"LedgerEntryType\":\"AccountRoot\",\"LedgerIndex\":\"12070BCA199E4D6AAECDBC9BB210D94805BDDBD179FDB9E05D4DDECA1E78208E\",\"PreviousFields\":{\"Balance\":\"1281979756\",\"Sequence\":969192},\"PreviousTxnID\":\"CB49B5F0BB791DEC950E847F191E0242C3B9CB727C7422969BBD0D86B32DC50A\",\"PreviousTxnLgrSeq\":44352209}}],\"TransactionIndex\":7,\"TransactionResult\":\"tecDST_TAG_NEEDED\"},\"status\":\"closed\",\"transaction\":{\"Account\":\"rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k\",\"Amount\":\"1000000\",\"Destination\":\"rEb8TK3gBgk5auZkwc6sHnwrGVJH8DuaLh\",\"Fee\":\"10\",\"Flags\":2147483648,\"Sequence\":969192,\"SigningPubKey\":\"03A9BD0A9223A32AC9DE972CA9ACDCBAFD29FE2C68AEC37E667CC862A71226A380\",\"TransactionType\":\"Payment\",\"TxnSignature\":\"3044022046BFA586F7439CB9C685FC04F7D2E5A38421380FB73718B08ABAE9FFA5AC64BF02203CBE01242DCF06A71FC04BF60890326ECB2D877FE729D11BC7A3E49B8272DF24\",\"date\":600610761,\"hash\":\"F89CBBDA47B4BABF541A7FFCB6A4D6905A4DBD48A5E6BE29B5EDD3DA8E18CFAF\"},\"type\":\"transaction\",\"validated\":true}";
TEST_F(RippleLikeWalletSynchronization, EmitNewTransactionAndReceiveOnPool) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        auto wallet = wait(pool->createWallet("e847815f-488a-4301", "ripple", configuration));
        auto account = createRippleLikeAccount(wallet, 0, XRP_KEYS_INFO);

        auto receiver = make_receiver([&] (const std::shared_ptr<api::Event>& event) {
            if (event->getCode() == api::EventCode::NEW_OPERATION) {
                EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
                dispatcher->stop();
            }
        });
        ws->setOnConnectCallback([&] () {
            ws->push(NOTIF_WITH_TX);
        });
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
        pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);
        account->startBlockchainObservation();
        dispatcher->waitUntilStopped();
    }
}

const std::string NOTIF_WITH_BLOCK = "{\"fee_base\":10,\"fee_ref\":10,\"ledger_hash\":\"43BF0F7D1131B5926153E8847CC42B8652B451DF09F94558BE8FF9FF9F846428\",\"ledger_index\":44351888,\"ledger_time\":600609550,\"reserve_base\":20000000,\"reserve_inc\":5000000,\"txn_count\":26,\"type\":\"ledgerClosed\",\"validated_ledgers\":\"32570-44351888\"}";
TEST_F(RippleLikeWalletSynchronization, EmitNewBlock) {
    auto pool = newDefaultPool();
    {
        auto wallet = wait(pool->createWallet("e847815f-488a-4301", "ripple", api::DynamicObject::newInstance()));
        auto account = createRippleLikeAccount(wallet, 0, XRP_KEYS_INFO);
        auto receiver = make_receiver([&] (const std::shared_ptr<api::Event>& event) {
            if (event->getCode() == api::EventCode::NEW_BLOCK) {
                try {
                    auto height = event->getPayload()->getLong(api::Account::EV_NEW_BLOCK_HEIGHT).value_or(0);
                    auto hash = event->getPayload()->getString(api::Account::EV_NEW_BLOCK_HASH).value_or("");
                    auto block = wait(pool->getLastBlock("ripple"));
                    EXPECT_EQ(height, block.height);
                    EXPECT_EQ(hash, block.blockHash);
                } catch (const std::exception& ex) {
                    fmt::print("{}", ex.what());
                    FAIL();
                }
                dispatcher->stop();
            }
        });
        ws->setOnConnectCallback([&] () {
            ws->push(NOTIF_WITH_BLOCK);
        });
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rMspb4Kxa3EwdF4uN5TMqhHfsAkBit6w7k");
        account->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);
        account->startBlockchainObservation();
        dispatcher->waitUntilStopped();
    }
}