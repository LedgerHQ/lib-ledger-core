/*
 *
 * tezos/SECP256K1_transaction_tests
 *
 * Created by Habib LAFET on 23/10/2020.
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
#include "transaction_test.hpp"
#include <api/DynamicObject.hpp>
#include <api/KeychainEngines.hpp>
#include <api/TezosLikeOriginatedAccount.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <api/BlockchainExplorerEngines.hpp>
#include <utils/hex.h>
#include <utils/DateUtils.hpp>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/currencies.hpp>
#include <iostream>
#include <wallet/tezos/tezosNetworks.h>

using namespace std;
using namespace ledger::testing::tezos;

struct SECP256K1TransactionTest : public TezosBaseTest {};

struct SECP256K1TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        std::cout << "SetUpConfig" << std::endl;
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "tezos";
        testData.inflate_xtz = inflate_SECP256K1;
    }
};

  TEST_F(SECP256K1TezosMakeTransaction, CreateTx) {
        /*
{'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BLGZq6X1oeSP8391ipH1CrosgozqKZjVgth6YoNwp1SGbsNa1Xf',
 'contents': [{'kind': 'transaction',
   'source': 'tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC',
   'fee': '1294',
   'counter': '7504849',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '5000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigRdZiyxWrBwxKvEJvZctMTgSkcu6F2bKvexmBV4tkUdHB44FcVkNBgcJJLL8NU4itx4aDfpuYS7VxLNir9aqiCXGWPdFZn'}

binary payload
'494e3f0ef57f41f7df28858ec359f707be99348e5627e5f975f523845c671b776c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba98e0ad187ca03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129001bd7ac6f01974cc9076b232a2eb6eb8462c5affee19c9eca3912b933f954d3a0252035989143792f66cad887d63dd2356985dbb9295e4929a724e26142d49d18'
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

      builder->setFees(api::Amount::fromLong(currency, 1294));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 5000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(::wait(f));
      tx->setBlockHash("BLGZq6X1oeSP8391ipH1CrosgozqKZjVgth6YoNwp1SGbsNa1Xf");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7504849")));
      tx->setSignature(hex::toByteArray("1bd7ac6f01974cc9076b232a2eb6eb8462c5affee19c9eca3912b933f954d3a0252035989143792f66cad887d63dd2356985dbb9295e4929a724e26142d49d18"));
      tx->reveal(false);

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload), "494e3f0ef57f41f7df28858ec359f707be99348e5627e5f975f523845c671b776c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba98e0ad187ca03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129001bd7ac6f01974cc9076b232a2eb6eb8462c5affee19c9eca3912b933f954d3a0252035989143792f66cad887d63dd2356985dbb9295e4929a724e26142d49d18");
    
      //broadcast(tx);    
 }

  TEST_F(SECP256K1TezosMakeTransaction, CreateTxWithReveal) {
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

