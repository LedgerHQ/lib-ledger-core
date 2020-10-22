/*
 *
 * tezos/transaction_tests
 *
 * Created by Axel Haustant on 10/09/2020.
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

#include "Fixtures.hpp"

// #include "../BaseFixture.h"
// #include "../../fixtures/xtz_fixtures.h"
#include <api/DynamicObject.hpp>
#include <api/KeychainEngines.hpp>
#include <api/TezosLikeOriginatedAccount.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <wallet/currencies.hpp>
#include <iostream>
#include <wallet/tezos/tezosNetworks.h>

using namespace std;
using namespace ledger::testing::tezos;

struct TransactionTest : public TezosBaseTest {};

INSTANTIATE_TEST_CASE_P(
    Tezos,
    TransactionTest,
    TezosParams(),
    TezosParamsNames
);

// struct TezosMakeTransaction : public TezosMakeBaseTransaction {};

struct TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        std::cout << "SetUpConfig" << std::endl;
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        //configuration->putString(api::TezosConfiguration::TEZOS_NODE, "https://xtz-node.api.live.ledger.com");
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "tezos";
        testData.inflate_xtz = inflate;
    }
    struct Callback: public api::StringCallback {
        Callback(std::shared_ptr<QtThreadDispatcher> dispatcher): _dispatcher(dispatcher)
        {}
        virtual void onCallback(const std::experimental::optional<std::string> & result, const std::experimental::optional<api::Error> & error) override {
            if (result) {
                std::cout << "broadcastTransaction callback result " << result.value() << std::endl;
            }
            if (error) {
                std::cout << "broadcastTransaction callback error " << error.value().code << "/" <<error.value().message << std::endl;
            }
            _dispatcher->stop();
        }
        private:
        std::shared_ptr<QtThreadDispatcher> _dispatcher;
    };

    void broadcast(std::shared_ptr<TezosLikeTransactionApi> tx) {
        dispatcher = std::make_shared<QtThreadDispatcher>();
        auto callback = std::make_shared<Callback>(dispatcher);  
        account->broadcastTransaction(tx, callback);
        dispatcher->waitUntilStopped();
    }
};

  TEST_F(TezosMakeTransaction, CreateTransactionTx) {
    /*
    {'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
    'branch': 'BL15xU2AmuVr6r6WkmeZgqAiLUvwcBfr23FR7vgFiNvmEmxYceY',
    'contents': [{'kind': 'transaction',
        'source': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD',
        'fee': '1294',
        'counter': '7314350',
        'gas_limit': '10407',
        'storage_limit': '0',
        'amount': '10000',
        'destination': 'tz1TbV2GhApnwtyxN5ETLiVUDi9R6QsBaAMC'}],
    'signature': 'sighERkE6XKCDaapo6vznSxefQCipLVB3E5CQZW5WvNbBRTvT2zBHSnftfie6bp1EaaLJzkuzzQWm4cfKn9zVxthxNc32v49'}

    binary payload:
    2627b66bb9978a341593431f4ce23089f0e10e048c36749af11432244d2da11a6c011ebab3538f6ca4223ee98b565846e47d273d11298e0aaeb7be03a75100904e00005745bcdc5b2478e4ae7b3e8f4589e4ae93490e7a0093173edeae0c7b15636b2127f08e48fb982d97803baa8aa6ac29dd0d98ab80ed2990c44971d30f0ee8f8fb514a2a50307e0c8cde644f346da758437c088af6a5
    */
     const std::string DESTINATION = "tz1TbV2GhApnwtyxN5ETLiVUDi9R6QsBaAMC"; 
     
     auto builder = tx_builder();
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;
     
          EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
          EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
          dispatcher->stop();
      });
      account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);
      dispatcher->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 1294));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 10000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(::wait(f));
      tx->setBlockHash("BL15xU2AmuVr6r6WkmeZgqAiLUvwcBfr23FR7vgFiNvmEmxYceY");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7314350")));
      tx->setSignature(hex::toByteArray(
          "93173edeae0c7b15636b2127f08e48fb982d97803baa8aa6ac29dd0d98ab80ed2990c44971d30f0ee8f8fb514a2a50307e0c8cde644f346da758437c088af6a5"));

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload),
        "2627b66bb9978a341593431f4ce23089f0e10e048c36749af11432244d2da11a6c011ebab3538f6ca4223ee98b565846e47d273d11298e0aaeb7be03a75100904e00005745bcdc5b2478e4ae7b3e8f4589e4ae93490e7a0093173edeae0c7b15636b2127f08e48fb982d97803baa8aa6ac29dd0d98ab80ed2990c44971d30f0ee8f8fb514a2a50307e0c8cde644f346da758437c088af6a5");
    
      //broadcast(tx);    
 }

  TEST_F(TezosMakeTransaction, CreateRevealTx) {
    /*
    json payload:
 {'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BLmyojLLzoc3H4Hfjpc1kXfwapD7Mxhg2Exeg7tdMaKWLDtPMBA',
 'contents': [{'kind': 'reveal',
   'source': 'tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC',
   'fee': '1234',
   'counter': '7504846',
   'gas_limit': '10200',
   'storage_limit': '0',
   'public_key': 'sppk7ZrJrq1qbRRzpXP2JfWuF9DhvgLpePj4c52oYmXLweCDWaydAGy'},
  {'kind': 'transaction',
   'source': 'tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC',
   'fee': '1247',
   'counter': '7504847',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '20000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigSTamqNS1kz177crX6d4N5QA1oBcLtP3L1RH6t8Ngd8yg8jALCEySuVwxdCuVEJYpnEKBAFG7gHJQVvoRo3wj35Mzz5se7'}

    binary payload:
    '8c17a3d65cb024febe2a869f7bb44469b85da14aa5a95892cd7001334b237a546b010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9d209ce87ca03d84f00010247ea904a9456e06232533a80a2c5bc9f50417755504025d8f556d259da66e4576c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9df09cf87ca03a75100a09c0100011ebab3538f6ca4223ee98b565846e47d273d112900222bac041fdcd983f5c100413b3be83ef0bf2b60c857a9dc7e6780c8644244af28038322d49f081e6286d93e4941589e595bfc7555f539f66b939f15993b77e9'
    */
     const std::string DESTINATION = "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD"; 

     auto builder = tx_builder();
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;
     
          EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
          EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
          dispatcher->stop();
      });
      account->synchronize()->subscribe(dispatcher->getMainExecutionContext(), receiver);
      dispatcher->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 1247));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 20000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(::wait(f));
      tx->setRevealFees(std::make_shared<BigInt>(BigInt::fromString("1234")));
      tx->setRevealGasLimit(std::make_shared<BigInt>(BigInt::fromString("10200")));
      tx->setBlockHash("BLmyojLLzoc3H4Hfjpc1kXfwapD7Mxhg2Exeg7tdMaKWLDtPMBA");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7504846")));
      tx->setSignature(hex::toByteArray("222bac041fdcd983f5c100413b3be83ef0bf2b60c857a9dc7e6780c8644244af28038322d49f081e6286d93e4941589e595bfc7555f539f66b939f15993b77e9"));
      tx->reveal(true);
      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload),
       "8c17a3d65cb024febe2a869f7bb44469b85da14aa5a95892cd7001334b237a546b010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9d209ce87ca03d84f00010247ea904a9456e06232533a80a2c5bc9f50417755504025d8f556d259da66e4576c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9df09cf87ca03a75100a09c0100011ebab3538f6ca4223ee98b565846e47d273d112900222bac041fdcd983f5c100413b3be83ef0bf2b60c857a9dc7e6780c8644244af28038322d49f081e6286d93e4941589e595bfc7555f539f66b939f15993b77e9");
      //broadcast(tx);    
 }

 

TEST_P(TransactionTest, ParseUnsignedRawTransaction) {
    // round-trip
    auto strTx = "032cd54a6d49c82da8807044a41f8670cf31832cb12c151fe2f605407bcdf558960800008bd703c4a2d91b8f1d79455be9b99c2693e931fdfa0901d84f950280c8afa0250000d2e495a7ab40156d0a7c35b73d2530a3470fc87000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    //EXPECT_EQ(tx->getHash(), "opBQwjhyCEcH5JxUtthPxh7m7sBxGKnCzVYwUcR7xr7MGwotZnW");
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YPSCGWXwBdTncK2aCctSZAXWvGsGwVJqU");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 10000000000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1274L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10200L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "277");
}

TEST_P(TransactionTest, ParseUnsignedRawRevealTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94070000d2e495a7ab40156d0a7c35b73d2530a3470fc870ea0902904e0000cda3081bd81219ec494b29068dcfd19e427fed9a66abcdc9e9e99ca6478f60e9";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1258L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10000L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_P(TransactionTest, ParseUnsignedRawOriginationTransaction) {
    // round-trip
    auto strTx = "03a43f08f2b1d38e7c2762fc1b123b3ab772ae34669c2b541a0f7e96a104341e94090000d2e495a7ab40156d0a7c35b73d2530a3470fc870920903f44e950200d2e495a7ab40156d0a7c35b73d2530a3470fc8708094ebdc03ffff0000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1es8RjqHUD483BN9APWtvCzgjTFVGeMh3y");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1170L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10100L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "277");
}

TEST_P(TransactionTest, ParseUnsignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "037d8d230a91d1fb8391727f37a1bfeb332b7f249c78315ea4ae934e2103a826630a01d315f72434520d43d415f0dff4632519501d2d9400890902f44e00ff008bd703c4a2d91b8f1d79455be9b99c2693e931fd";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "KT1TptTRYx2BEYetm61ABjkBdfHXQ2SQrXo8");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1YPSCGWXwBdTncK2aCctSZAXWvGsGwVJqU");
    EXPECT_EQ(tx->getValue()->toLong(), 0L);
    EXPECT_EQ(tx->getFees()->toLong(), 1161L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10100L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

// Reference https://tzscan.io/ooPbtVVy7TZLoRirGsCgyy6Esyqm3Kj22QvEVpAmEXX3vHBGbF8
TEST_P(TransactionTest, ParseSignedRawDelegationTransaction) {
    // round-trip
    auto strTx = "4fd4dca725498e819cc4dd6a87adcfb98770f600558d5d097d1a48b2324a9a2e080000bbdd4268871d1751a601fe66603324714266bf558c0bdeff4ebc50ac02a08d0601583f106387cb85212812b738cae45b497551bf9a00007dc21f46b94d6b432c881b78e1fee917ef0fd382571a5d5f1bf1c5aa90d62a02777eeebac53e3fe9bbcf4501cc2ee0cb1dbe65ec24c869a4715d84f65cfdc101";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = api::TezosLikeTransactionBuilder::parseRawSignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);

    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "KT1GdNaQowD3r8VprK8pni2R2DZd5Vxnkvw5");
    // It is BLKSXLjQkkPj3WsRZQ6PAwDotSbR25t3mKvZ7bvXUx75Poex4u9 predecessor of
    // BM7DAEUj8dCNUzViXjg8DyeWtthvorDXMk6ocMwNz7iVo73WNKC where tx included
    EXPECT_EQ(tx->getBlockHash().value(), "BLKSXLjQkkPj3WsRZQ6PAwDotSbR25t3mKvZ7bvXUx75Poex4u9");
    EXPECT_EQ(tx->getValue()->toLong(), 100000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1420L);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10300L);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "300");
    EXPECT_EQ(tx->getCounter()->toString(10), "1294302");
    

}
