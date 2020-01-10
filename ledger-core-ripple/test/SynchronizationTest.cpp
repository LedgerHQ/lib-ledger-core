#include <gtest/gtest.h>
#include <iostream>
#include <set>

#include <core/api/BlockchainExplorerEngines.hpp>
#include <core/api/KeychainEngines.hpp>
#include <core/utils/DateUtils.hpp>
#include <ripple/RippleLikeAccount.hpp>
#include <ripple/RippleLikeAccountDatabaseHelper.hpp>
#include <ripple/RippleLikeCurrencies.hpp>
#include <ripple/RippleLikeOperationQuery.hpp>
#include <ripple/RippleLikeWallet.hpp>
#include <ripple/api/RippleLikeOperation.hpp>
#include <ripple/api/RippleLikeTransaction.hpp>
#include <ripple/factories/RippleLikeWalletFactory.hpp>
#include <ripple/transaction_builders/RippleLikeTransactionBuilder.hpp>

#include <async/AsyncWait.hpp>
#include <integration/BaseFixture.hpp>

#include "Common.hpp"

using namespace std;

struct RippleLikeWalletSynchronization : BaseFixture {
};

TEST_F(RippleLikeWalletSynchronization, MediumXpubSynchronization) {
    auto services = newDefaultServices();
    auto walletStore = newWalletStore(services);
    walletStore->addCurrency(currencies::ripple());

    auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
    walletStore->registerFactory(currencies::ripple(), factory);

    {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                                 "44'/<coin_type>'/<account>'/<node>/<address>");

        auto wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration)));

        std::set<std::string> emittedOperations;
        {
            auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);

            auto info = XRP_KEYS_INFO;
            info.index = nextIndex;
            auto account = std::dynamic_pointer_cast<RippleLikeAccount>(wait(wallet->newAccountWithInfo(info)));

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

            services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);

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

            auto ops = wait(std::dynamic_pointer_cast<RippleLikeOperationQuery>(account->queryOperations()->complete())->execute());
            std::cout << "Ops: " << ops.size() << std::endl;

            for (auto const& op : ops) {
                auto xrpOp = std::dynamic_pointer_cast<RippleLikeOperation>(op);
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

const std::string NOTIF_WITH_TX = "{\"engine_result\":\"tesSUCCESS\",\"engine_result_code\":1,\"engine_result_message\":\"A destination tag is required.\",\"ledger_hash\":\"42A6250BE0CED050AD5AA7858B9D8F53E2F39377525C04B98DBB62F9321AB176\",\"ledger_index\":50394479,\"meta\":{\"AffectedNodes\":[{\"ModifiedNode\":{\"FinalFields\":{\"Account\":\"rJXvTXRLvQVhLGLaBsLE8JEFzRvNs9SY5e\",\"Balance\":\"30457198\",\"Flags\":0,\"OwnerCount\":0,\"Sequence\":72},\"LedgerEntryType\":\"AccountRoot\",\"LedgerIndex\":\"77C8BE5F5FBBFA60CB291BDE1BF0D76D961CD427539CCEF7F39F1467112F9518\",\"PreviousFields\":{\"Balance\":\"30457209\",\"Sequence\":71},\"PreviousTxnID\":\"91B5AF6E6CDA92DE8AADF6A8ABE17E0183A9A4A6CBFC1D4182D03355972E00C6\",\"PreviousTxnLgrSeq\":50394473}}],\"TransactionIndex\":12,\"TransactionResult\":\"tesSUCCESS\"},\"status\":\"success\",\"transaction\":{\"Account\":\"rJXvTXRLvQVhLGLaBsLE8JEFzRvNs9SY5e\",\"Amount\":\"1\",\"Destination\":\"rageXHB6Q4VbvvWdTzKANwjeCT4HXFCKX7\",\"Fee\":\"10\",\"Flags\":2147483648,\"Sequence\":71,\"SigningPubKey\":\"028EB02B5AEB00B704953BB1075E03AB88B34FCF38A256A0E62A7CEE5F246976E4\",\"TransactionType\":\"Payment\",\"TxnSignature\":\"3045022100E0A428A6C3F123591063C10220744026D2BE154BE00039797347C5AAAF70FF4702203843347E2CF6700536AC7236CD455AF76225FA9D5B55A1E7A1C1CE580047B482\",\"date\":623185870,\"hash\":\"AC0D84CB81E8ECA92E7EF9ABC3526FAED54DE07763A308296B28468D68D34991\"},\"type\":\"transaction\",\"validated\":true}";
TEST_F(RippleLikeWalletSynchronization, EmitNewTransactionAndReceiveOnPool) {
    auto services = newDefaultServices();
    {
        auto configuration = DynamicObject::newInstance();

        auto walletStore = newWalletStore(services);
        walletStore->addCurrency(currencies::ripple());

        auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
        walletStore->registerFactory(currencies::ripple(), factory);

        auto wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet("e847815f-488a-4301", "ripple", configuration)));
        auto account = std::dynamic_pointer_cast<RippleLikeAccount>(wait(wallet->newAccountWithInfo(XRP_KEYS_INFO)));

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
        services->getEventBus()->subscribe(dispatcher->getMainExecutionContext(), receiver);
        account->startBlockchainObservation();
        dispatcher->waitUntilStopped();
    }
}

const std::string NOTIF_WITH_BLOCK = "{\"fee_base\":10,\"fee_ref\":10,\"ledger_hash\":\"43BF0F7D1131B5926153E8847CC42B8652B451DF09F94558BE8FF9FF9F846428\",\"ledger_index\":44351888,\"ledger_time\":600609550,\"reserve_base\":20000000,\"reserve_inc\":5000000,\"txn_count\":26,\"type\":\"ledgerClosed\",\"validated_ledgers\":\"32570-44351888\"}";
TEST_F(RippleLikeWalletSynchronization, EmitNewBlock) {
    auto services = newDefaultServices();
    {
        auto configuration = DynamicObject::newInstance();

        auto walletStore = newWalletStore(services);
        walletStore->addCurrency(currencies::ripple());

        auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
        walletStore->registerFactory(currencies::ripple(), factory);

        auto wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet("e847815f-488a-4301", "ripple", configuration)));
        auto account = std::dynamic_pointer_cast<RippleLikeAccount>(wait(wallet->newAccountWithInfo(XRP_KEYS_INFO)));
        auto receiver = make_receiver([&] (const std::shared_ptr<api::Event>& event) {
            if (event->getCode() == api::EventCode::NEW_BLOCK) {
                try {
                    auto height = event->getPayload()->getLong(api::Account::EV_NEW_BLOCK_HEIGHT).value_or(0);
                    auto hash = event->getPayload()->getString(api::Account::EV_NEW_BLOCK_HASH).value_or("");
                    auto block = wait(services->getLastBlock("ripple"));
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
    auto services = newDefaultServices();
    auto configuration = DynamicObject::newInstance();
    configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,
                             "44'/<coin_type>'/<account>'/<node>/<address>");

    auto walletStore = newWalletStore(services);
    walletStore->addCurrency(currencies::ripple());

    auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
    walletStore->registerFactory(currencies::ripple(), factory);

    auto wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "ripple", configuration)));

    auto nextIndex = wait(wallet->getNextAccountIndex());
    auto info = VAULT_XRP_KEYS_INFO;
    info.index = nextIndex;
    auto account = std::dynamic_pointer_cast<RippleLikeAccount>(wait(wallet->newAccountWithInfo(info)));
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
            std::dynamic_pointer_cast<RippleLikeOperationQuery>(account->queryOperations()->complete())->execute());
    std::cout << "Ops: " << ops.size() << std::endl;

    uint32_t destinationTag = 0;
    for (auto const& op : ops) {
        auto xrpOp = std::dynamic_pointer_cast<RippleLikeOperation>(op);
        if (xrpOp->getTransaction()->getHash() == "609AC2E159F521563374383DEB3D57E098F7A2CC9481F02F4642D1F74CB2F3C9") {
          destinationTag = xrpOp->getTransaction()->getDestinationTag().value_or(0);
          break;
        }
    }

    EXPECT_TRUE(destinationTag == 123456789);
}
