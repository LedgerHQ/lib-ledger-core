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
#include <api/RippleLikeOperation.hpp>
#include <api/RippleLikeTransaction.hpp>

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

            for (auto const& op : ops) {
                auto xrpOp = op->asRippleLikeOperation();
                EXPECT_FALSE(xrpOp == nullptr);
                EXPECT_FALSE(xrpOp->getTransaction()->getSequence() == nullptr);
                EXPECT_TRUE(std::chrono::duration_cast<std::chrono::hours>(xrpOp->getTransaction()->getDate().time_since_epoch()).count() != 0);
            }

            auto block = wait(account->getLastBlock());
            auto blockHash = block.blockHash;

            EXPECT_EQ(wait(account->isAddressActivated("rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7")), true);
            EXPECT_EQ(wait(account->isAddressActivated("rageXHB6Q4VbvvWdTzKANwjeCT4HXFCK")), false);
            EXPECT_EQ(wait(account->isAddressActivated("rf1pjatD8LyyevP1BqQJtHoz5edC5vE77Q")), false);
        }
    }
}

TEST_F(RippleLikeWalletSynchronization, BalanceHistory) {
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

            auto receiver = make_receiver([&](const std::shared_ptr<api::Event> &event) {
                if (event->getCode() == api::EventCode::NEW_OPERATION) {
                    auto uid = event->getPayload()->getString(
                            api::Account::EV_NEW_OP_UID).value();
                    EXPECT_EQ(emittedOperations.find(uid), emittedOperations.end());
                }
            });

            pool->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            std::shared_ptr<Amount> balance;

            receiver.reset();
            receiver = make_receiver([=, &balance](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
                EXPECT_EQ(event->getCode(),
                          api::EventCode::SYNCHRONIZATION_SUCCEED);

                balance = wait(account->getBalance());
                std::cout << "Balance: " << balance->toString() << std::endl;
                auto txBuilder = std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
                dispatcher->stop();
            });

            auto restoreKey = account->getRestoreKey();
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);

            dispatcher->waitUntilStopped();

            auto now = std::time(nullptr);
            char now_str[256];
            std::strftime(now_str, sizeof(now_str), "%Y-%m-%dT%H:%M:%SZ", std::localtime(&now));

            auto history = wait(account->getBalanceHistory(
                "2019-09-20T00:00:00Z",
                now_str,
                api::TimePeriod::DAY
            ));

            EXPECT_EQ(history.back()->toString(), balance->toString());

            auto zero = std::make_shared<api::BigIntImpl>(BigInt::ZERO);
            for (auto const& balance : history) {
                EXPECT_TRUE(balance->toBigInt()->compare(zero) > 0);
            }
        }
    }
}

const std::string NOTIF_WITH_TX = "{\"engine_result\":\"tesSUCCESS\",\"engine_result_code\":1,\"engine_result_message\":\"A destination tag is required.\",\"ledger_hash\":\"42A6250BE0CED050AD5AA7858B9D8F53E2F39377525C04B98DBB62F9321AB176\",\"ledger_index\":50394479,\"meta\":{\"AffectedNodes\":[{\"ModifiedNode\":{\"FinalFields\":{\"Account\":\"rJXvTXRLvQVhLGLaBsLE8JEFzRvNs9SY5e\",\"Balance\":\"30457198\",\"Flags\":0,\"OwnerCount\":0,\"Sequence\":72},\"LedgerEntryType\":\"AccountRoot\",\"LedgerIndex\":\"77C8BE5F5FBBFA60CB291BDE1BF0D76D961CD427539CCEF7F39F1467112F9518\",\"PreviousFields\":{\"Balance\":\"30457209\",\"Sequence\":71},\"PreviousTxnID\":\"91B5AF6E6CDA92DE8AADF6A8ABE17E0183A9A4A6CBFC1D4182D03355972E00C6\",\"PreviousTxnLgrSeq\":50394473}}],\"TransactionIndex\":12,\"TransactionResult\":\"tesSUCCESS\"},\"status\":\"success\",\"transaction\":{\"Account\":\"rJXvTXRLvQVhLGLaBsLE8JEFzRvNs9SY5e\",\"Amount\":\"1\",\"Destination\":\"rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7\",\"Fee\":\"10\",\"Flags\":2147483648,\"Sequence\":71,\"SigningPubKey\":\"028EB02B5AEB00B704953BB1075E03AB88B34FCF38A256A0E62A7CEE5F246976E4\",\"TransactionType\":\"Payment\",\"TxnSignature\":\"3045022100E0A428A6C3F123591063C10220744026D2BE154BE00039797347C5AAAF70FF4702203843347E2CF6700536AC7236CD455AF76225FA9D5B55A1E7A1C1CE580047B482\",\"date\":623185870,\"hash\":\"AC0D84CB81E8ECA92E7EF9ABC3526FAED54DE07763A308296B28468D68D34991\"},\"type\":\"transaction\",\"validated\":true}";
TEST_F(RippleLikeWalletSynchronization, EmitNewTransactionAndReceiveOnPool) {
    auto pool = newDefaultPool();
    {
        auto configuration = DynamicObject::newInstance();
        auto wallet = wait(pool->createWallet("e847815f-488a-4301", "ripple", configuration));
        auto account = createRippleLikeAccount(wallet, 0, XRP_KEYS_INFO);

        auto receiver = make_receiver([&] (const std::shared_ptr<api::Event>& event) {
            if (event->getCode() == api::EventCode::NEW_OPERATION) {
                EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7");
                dispatcher->stop();
            }
        });
        ws->setOnConnectCallback([&] () {
            ws->push(NOTIF_WITH_TX);
        });
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7");
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
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), "rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7");
        account->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);
        account->startBlockchainObservation();
        dispatcher->waitUntilStopped();
    }
}

TEST_F(RippleLikeWalletSynchronization, VaultAccountSynchronization) {
    auto pool = newDefaultPool();
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");
    auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration));
    auto nextIndex = wait(wallet->getNextAccountIndex());
    auto account = createRippleLikeAccount(wallet, nextIndex, VAULT_XRP_KEYS_INFO);
    auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
            return;
        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(),
                  api::EventCode::SYNCHRONIZATION_SUCCEED);

        dispatcher->stop();
    });

    account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);
    dispatcher->waitUntilStopped();

    auto ops = wait(
            std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
    std::cout << "Ops: " << ops.size() << std::endl;

    uint32_t destinationTag = 0;
    for (auto const& op : ops) {
        auto xrpOp = op->asRippleLikeOperation();

        if (xrpOp->getTransaction()->getHash() == "EE38840B83CAB39216611D2F6E4F9828818514C3EA47504AE2521D8957331D3C" ) {
          destinationTag = xrpOp->getTransaction()->getDestinationTag().value_or(0);
          break;
        }
    }

    EXPECT_TRUE(destinationTag == 123456789);
}
