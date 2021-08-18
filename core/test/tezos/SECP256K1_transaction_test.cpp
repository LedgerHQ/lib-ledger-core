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

struct SECP256K1TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::TezosConfiguration::TEZOS_NODE, "https://xtz-node.api.live.ledger.com");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_SECP256K1);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        configuration->putString(api::TezosConfiguration::TEZOS_COUNTER_STRATEGY, "OPTIMISTIC");
        testData.configuration = configuration;
        testData.walletName = randomWalletName();
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
          getTestExecutionContext()->stop();
      });
      auto bus = account->synchronize();
      bus->subscribe(getTestExecutionContext(), receiver);
      getTestExecutionContext()->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 1294));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 5000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
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
          getTestExecutionContext()->stop();
      });
      auto bus = account->synchronize();
      bus->subscribe(getTestExecutionContext(), receiver);
      getTestExecutionContext()->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 1247));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 20000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
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

TEST_F(SECP256K1TezosMakeTransaction, ParseUnsignedRawTransaction) {
    auto strTx = "03494e3f0ef57f41f7df28858ec359f707be99348e5627e5f975f523845c671b776c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba98e0ad187ca03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD");
    EXPECT_EQ(tx->getValue()->toLong(), 5000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1294);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10407);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(SECP256K1TezosMakeTransaction, ParseUnsignedRawTransactionWithReveal) {
    auto strTx = "038c17a3d65cb024febe2a869f7bb44469b85da14aa5a95892cd7001334b237a546b010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9d209ce87ca03d84f00010247ea904a9456e06232533a80a2c5bc9f50417755504025d8f556d259da66e4576c010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9df09cf87ca03a75100a09c0100011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    //tx->setRawTx(std::vector<unsigned char>{});
    //EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD");
    EXPECT_EQ(tx->getValue()->toLong(), 20000);
    EXPECT_EQ(tx->toReveal(), true);
    EXPECT_EQ(tx->getFees()->toLong(), 1247+1234); //sum of transaction fees and reveal fees
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10407+10200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

  TEST_F(SECP256K1TezosMakeTransaction, DISABLED_optimisticCounter) {
    
     const std::string DESTINATION = "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD"; 
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;

        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
        getTestExecutionContext()->stop();
    });
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);
    getTestExecutionContext()->waitUntilStopped();

     auto createTx = [&] () {
        auto builder = tx_builder();

        builder->setFees(api::Amount::fromLong(currency, 1294));
        builder->setGasLimit(api::Amount::fromLong(currency, 10407));
        builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
        builder->sendToAddress(api::Amount::fromLong(currency, 5000), DESTINATION);
        auto f = builder->build();
        auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
        return tx;
      };
  
      std::cout << "starting transaction 1" << std::endl;
      auto tx1 = createTx();
      auto counter1 = tx1->getCounter();
      std::cout << "counter1=" << counter1->intValue() << std::endl;
      
      std::cout << "starting transaction 2" << std::endl;
      auto tx2 = createTx();
      auto counter2 = tx2->getCounter();
      std::cout << "counter2=" << counter2->intValue() << std::endl;
      EXPECT_EQ(counter2->intValue(), counter1->intValue());
      //broadcast(tx2);

      std::cout << "starting transaction 3" << std::endl;
      auto tx3 = createTx();
      auto counter3 = tx3->getCounter();
      std::cout << "counter3=" <<counter3->intValue() << std::endl;
      EXPECT_EQ(counter3->intValue(), counter2->intValue() + 1);
      //broadcast(tx3);
      
 }

 TEST_F(SECP256K1TezosMakeTransaction, incrementOptimisticCounter) {
      const std::string DESTINATION = "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD"; 
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;

        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
        getTestExecutionContext()->stop();
    });
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);
    getTestExecutionContext()->waitUntilStopped();

    auto builder = tx_builder();
    builder->setFees(api::Amount::fromLong(currency, 1294));
    builder->setGasLimit(api::Amount::fromLong(currency, 10407));
    builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
    builder->sendToAddress(api::Amount::fromLong(currency, 5000), DESTINATION);
    auto f = builder->build();
  
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
    auto explorerCounter = std::make_shared<BigInt>(tx->getCounter()->toString(10));
    //test 1
    account->incrementOptimisticCounter(tx, explorerCounter);
    auto counter1 = tx->getCounter();
    std::cout << "counter1=" <<counter1->intValue() << std::endl;
    EXPECT_EQ(counter1->intValue(), explorerCounter->toInt64()+1);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash1"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

    //test 2  
    account->incrementOptimisticCounter(tx, explorerCounter);
    auto counter2 = tx->getCounter();
    std::cout << "counter2=" <<counter2->intValue() << std::endl;
    EXPECT_EQ(counter2->intValue(), explorerCounter->toInt64()+2);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash2"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

    //test 3
    account->incrementOptimisticCounter(tx, explorerCounter);
    auto counter3 = tx->getCounter();
    std::cout << "counter3=" <<counter3->intValue() << std::endl;
    EXPECT_EQ(counter3->intValue(), explorerCounter->toInt64()+3);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash3"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

    //test 4
    account->incrementOptimisticCounter(tx, std::make_shared<BigInt>(explorerCounter->toInt64()+1));
    auto counter4 = tx->getCounter();
    std::cout << "counter4=" <<counter4->intValue() << std::endl;
    EXPECT_EQ(counter4->intValue(), explorerCounter->toInt64()+4);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash4"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

    //test 5
    account->incrementOptimisticCounter(tx, std::make_shared<BigInt>(explorerCounter->toInt64()+4));
    auto counter5 = tx->getCounter();
    std::cout << "counter5=" <<counter5->intValue() << std::endl;
    EXPECT_EQ(counter5->intValue(), explorerCounter->toInt64()+5);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash5"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

    //test 6
    account->incrementOptimisticCounter(tx, std::make_shared<BigInt>(explorerCounter->toInt64()+4));
    auto counter6 = tx->getCounter();
    std::cout << "counter6=" <<counter6->intValue() << std::endl;
    EXPECT_EQ(counter6->intValue(), explorerCounter->toInt64()+6);
    account->saveOptimisticCounter(std::make_shared<BigInt>(tx->getCounter()->toString(10)), "hash6"); //to simulate broadcast
    std::cout << "================================"<< std::endl;

 }


  TEST_F(SECP256K1TezosMakeTransaction, CreateDelegation) {
        /*
        {'protocol': 'PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo',
        'branch': 'BMZQK6HEcwtSqqdwRfFjhjiZ743LvvaPWa5aLyxDEuzWw4DXSG3',
        'contents': [{'kind': 'delegation',
        'source': 'tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC',
        'fee': '370',
        'counter': '7504849',
        'gas_limit': '1200',
        'storage_limit': '0',
        'delegate': 'tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM'}],
        'signature': 'sigcdy2gpGFM8BKoUHbPj22RY478w5M28orR1ao3ksLN8TX47uy2zMhBmQYDQNV4wCKTTQ1EqDWwUSKyTa6J6GR2nRg9THmi'}

        'f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae'
    */
    
     const std::string DESTINATION = "tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM"; 
     
     auto builder = tx_builder();
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;
     
          EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
          EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
          getTestExecutionContext()->stop();
      });
      auto bus = account->synchronize();
      bus->subscribe(getTestExecutionContext(), receiver);
      getTestExecutionContext()->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 370));
      builder->setGasLimit(api::Amount::fromLong(currency, 1200));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->wipeToAddress(DESTINATION);

      builder->setType(api::TezosOperationTag::OPERATION_TAG_DELEGATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
      tx->setBlockHash("BMZQK6HEcwtSqqdwRfFjhjiZ743LvvaPWa5aLyxDEuzWw4DXSG3");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7504849")));
      tx->setSignature(hex::toByteArray("6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae"));
      tx->reveal(false);

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateDelegation - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload), "f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae");
      //broadcast(tx);    
 }

  TEST_F(SECP256K1TezosMakeTransaction, CreateUndelegation) {
        /*
        {'protocol': 'PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo',
        'branch': 'BMZQK6HEcwtSqqdwRfFjhjiZ743LvvaPWa5aLyxDEuzWw4DXSG3',
        'contents': [{'kind': 'delegation',
        'source': 'tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC',
        'fee': '370',
        'counter': '7504849',
        'gas_limit': '1200',
        'storage_limit': '0',
        'delegate': 'tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM'}],
        'signature': 'sigcdy2gpGFM8BKoUHbPj22RY478w5M28orR1ao3ksLN8TX47uy2zMhBmQYDQNV4wCKTTQ1EqDWwUSKyTa6J6GR2nRg9THmi'}

        'f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae'
    */
    
     const std::string DESTINATION = "tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM"; 
     
     auto builder = tx_builder();
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;
     
          EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
          EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
          getTestExecutionContext()->stop();
      });
      auto bus = account->synchronize();
      bus->subscribe(getTestExecutionContext(), receiver);
      getTestExecutionContext()->waitUntilStopped();

      builder->setFees(api::Amount::fromLong(currency, 370));
      builder->setGasLimit(api::Amount::fromLong(currency, 1200));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      //builder->wipeToAddress(DESTINATION);

      builder->setType(api::TezosOperationTag::OPERATION_TAG_DELEGATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
      tx->setBlockHash("BMZQK6HEcwtSqqdwRfFjhjiZ743LvvaPWa5aLyxDEuzWw4DXSG3");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7504849")));
      tx->setSignature(hex::toByteArray("6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae"));
      tx->reveal(false);

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateDelegation - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      //EXPECT_EQ(hex::toString(binaryPayload), "f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df6ff998ad44e9f50df24a7046d65526bccc3563c42dcfcc28ce554447764d738669a455a881e99f4d15989eaeb58e6002abaf696871b192b4d0fb73e4d70d2bae");
      //broadcast(tx);    
 }

TEST_F(SECP256K1TezosMakeTransaction, ParseUnsignedRawDelegation) {
    auto strTx = "03f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM");
    EXPECT_EQ(tx->getFees()->toLong(), 370);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 1200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(SECP256K1TezosMakeTransaction, ParseUnsignedRawUndelegation) {
    auto strTx = "03f33acfe3e5301a413daf6ba4ab424831c3ed731e1f3d2e41dd4d5581342e9aac6e010ac10c14347edb22b0fd9ae0b2e397ac7adfcba9f202d187ca03b0090000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC");
    EXPECT_EQ(tx->getReceiver(), nullptr);
    EXPECT_EQ(tx->getFees()->toLong(), 370);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 1200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
} 

TEST_F(SECP256K1TezosMakeTransaction, GetCurrentDelegation) {
   auto builder = tx_builder();
   auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
        fmt::print("Received event {}\n", api::to_string(event->getCode()));
        if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;

        EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
        EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
        getTestExecutionContext()->stop();
    });
    auto bus = account->synchronize();
    bus->subscribe(getTestExecutionContext(), receiver);
    getTestExecutionContext()->waitUntilStopped();

    auto delegate = uv::wait(account->getCurrentDelegate());
    EXPECT_EQ(delegate, "tz29J6gQdA4Y9Qi5AhNZGMhQUpr9TwLPNByC");
}

TEST_F(SECP256K1TezosMakeTransaction, ParseUnsignedRawDelegationWithReveal) {
    auto strTx = "031275CE3B5655A5F3823EA05E1249B0014EC9C12DC3AB3E7A558432C1C5D7BE916B01202508B1C8C3127642F3685465730A93E811108A64F1E5FB03640001037A8EA0E40DCDD4CA436A00465273EC189F2920B497014DAFA5FA52011E14381F6E01202508B1C8C3127642F3685465730A93E811108A64F2E5FB036464FF0183CF5677B85B378422F3A8F5D6F6CB238394B741";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    
    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz2BFCee4VSARxdc6Tv7asSiYZBF957e4cwd");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz2LLBZYevBRjNBvzJ24GbAkJ5bNFDQi3KQv");
    EXPECT_EQ(tx->getFees()->toLong(), 200);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "100");
    EXPECT_EQ(tx->toReveal(), true);
}

TEST_F(SECP256K1TezosMakeTransaction, CreateTxAutoFill) {
     const std::string DESTINATION = "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD"; 

     auto builder = tx_builder();
     auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
          fmt::print("Received event {}\n", api::to_string(event->getCode()));
          if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED) return;
     
          EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);
          EXPECT_EQ(event->getCode(), api::EventCode::SYNCHRONIZATION_SUCCEED);
          getTestExecutionContext()->stop();
      });
      auto bus = account->synchronize();
      bus->subscribe(getTestExecutionContext(), receiver);
      getTestExecutionContext()->waitUntilStopped();
      builder->setFees(api::Amount::fromLong(currency, 0));
      builder->setGasLimit(api::Amount::fromLong(currency, 0));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->wipeToAddress(DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(uv::wait(f));
      //tx->reveal(true);
      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;
      std::cout << "gasLimit= " << tx->getGasLimit()->toLong() << std::endl;
      std::cout << "fees= " << tx->getFees()->toLong() << std::endl;
      std::cout << "value= " << tx->getValue()->toLong() << std::endl;

      EXPECT_NE(tx->getGasLimit()->toLong(), 0);
      EXPECT_NE(tx->getFees()->toLong(), 0);
      EXPECT_EQ(tx->getFees()->toLong(), tx->getTransactionFees()->toLong());
      EXPECT_EQ(tx->getRevealFees()->toLong(), 0);
 }

