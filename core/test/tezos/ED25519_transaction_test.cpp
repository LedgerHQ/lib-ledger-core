/*
 *
 * tezos/ED25519_transaction_test
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

struct ED25519TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        std::cout << "SetUpConfig" << std::endl;
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "tezos";
        testData.inflate_xtz = inflate_ED25519;
    }
};

  TEST_F(ED25519TezosMakeTransaction, CreateTx) {
    /*
 {'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BLYvxMcCanoLsdbatFNJf3XCTJHGW2Autgi2R1dnczTczFi1VrK',
 'contents': [{'kind': 'transaction',
   'source': 'tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF',
   'fee': '1294',
   'counter': '7522926',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '5000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigQWhnp1EwxZz2AYy5qL27pJ1PnvePDfhpHS4MLdKUbh13nGWUZbMn7RdHeZhNK16oRqiioTYq94uUAmtdCnGuumMzikF29'}

In [52]: y.binary_payload().hex()[-128:]
Out[52]: '134b71aa4e57ad494a0eb069029e94c6a12f1fcf7400f365cd8bb9bdc8328d0f00a8adfa99fd78da4275495fb862560a0fbc1203d841dbbbdefdf1a6b60e3104'

In [53]: y.binary_payload().hex()
Out[53]: '6e766ee0733ef0fb6385f2034cfbd437247afad4b301ebce1b929a67ce4a0b8d6c00902c5d86590a2452f0ccf9c1fa55ae679de27d398e0aee94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900134b71aa4e57ad494a0eb069029e94c6a12f1fcf7400f365cd8bb9bdc8328d0f00a8adfa99fd78da4275495fb862560a0fbc1203d841dbbbdefdf1a6b60e3104'

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
      tx->setBlockHash("BLYvxMcCanoLsdbatFNJf3XCTJHGW2Autgi2R1dnczTczFi1VrK");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7522926")));
      tx->setSignature(hex::toByteArray("134b71aa4e57ad494a0eb069029e94c6a12f1fcf7400f365cd8bb9bdc8328d0f00a8adfa99fd78da4275495fb862560a0fbc1203d841dbbbdefdf1a6b60e3104"));
      tx->reveal(false);

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload),"6e766ee0733ef0fb6385f2034cfbd437247afad4b301ebce1b929a67ce4a0b8d6c00902c5d86590a2452f0ccf9c1fa55ae679de27d398e0aee94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900134b71aa4e57ad494a0eb069029e94c6a12f1fcf7400f365cd8bb9bdc8328d0f00a8adfa99fd78da4275495fb862560a0fbc1203d841dbbbdefdf1a6b60e3104");
      //broadcast(tx);
 }

  TEST_F(ED25519TezosMakeTransaction, CreateTxWithReveal) {
    /*
  
In [24]: y.json_payload()
Out[24]:
{'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BM2sbcVTnvoMGb3CCnFowVdt3wVGMuP1g5fMQvCGBUZRomvy1R1',
 'contents': [{'kind': 'reveal',
   'source': 'tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF',
   'fee': '1233',
   'counter': '7522924',
   'gas_limit': '10200',
   'storage_limit': '0',
   'public_key': 'edpkvZLkbFgjXLTqa193cDmrsWAo5Q7upHZMxacrobhQZUyTVU4ELs'},
  {'kind': 'transaction',
   'source': 'tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF',
   'fee': '1246',
   'counter': '7522925',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '5000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigbBFrWxzYjTNt9aiyZCPeimJ7t7hBHxrqDWhmybzNu2PaRstQuL9mffYVbB25mC5P7r5YojfzJARHeRuENNjWx5qtGRYpq'}

In [25]: y.binary_payload().hex()[-128:]
Out[25]: '64cfb76c4bdc6f103dea229696797be4b8eeedbdf1ab7ceefda3f72660b5d4b008ef1b77ba5fe64ffc49e3e971d9886aac4dff282d1b7993aeb328c02a7a030e'

In [26]: y.binary_payload().hex()
Out[26]: 'ade89a7e5460a33ebabcab7cb7b6589cd7e48b512ebdc57e0acfc439934b04096b00902c5d86590a2452f0ccf9c1fa55ae679de27d39d109ec94cb03d84f0000fc4e4f37ded512650ca89dffe6c622a96a20f375d56331174c17c7b25f3775c76c00902c5d86590a2452f0ccf9c1fa55ae679de27d39de09ed94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d11290064cfb76c4bdc6f103dea229696797be4b8eeedbdf1ab7ceefda3f72660b5d4b008ef1b77ba5fe64ffc49e3e971d9886aac4dff282d1b7993aeb328c02a7a030e'
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

      builder->setFees(api::Amount::fromLong(currency, 1246));
      builder->setGasLimit(api::Amount::fromLong(currency, 10407));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->sendToAddress(api::Amount::fromLong(currency, 5000), DESTINATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(::wait(f));
      tx->setRevealFees(std::make_shared<BigInt>(BigInt::fromString("1233")));
      tx->setRevealGasLimit(std::make_shared<BigInt>(BigInt::fromString("10200")));
      tx->setBlockHash("BM2sbcVTnvoMGb3CCnFowVdt3wVGMuP1g5fMQvCGBUZRomvy1R1");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7522924")));
      tx->setSignature(hex::toByteArray("64cfb76c4bdc6f103dea229696797be4b8eeedbdf1ab7ceefda3f72660b5d4b008ef1b77ba5fe64ffc49e3e971d9886aac4dff282d1b7993aeb328c02a7a030e"));
      tx->reveal(true);
      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload),"ade89a7e5460a33ebabcab7cb7b6589cd7e48b512ebdc57e0acfc439934b04096b00902c5d86590a2452f0ccf9c1fa55ae679de27d39d109ec94cb03d84f0000fc4e4f37ded512650ca89dffe6c622a96a20f375d56331174c17c7b25f3775c76c00902c5d86590a2452f0ccf9c1fa55ae679de27d39de09ed94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d11290064cfb76c4bdc6f103dea229696797be4b8eeedbdf1ab7ceefda3f72660b5d4b008ef1b77ba5fe64ffc49e3e971d9886aac4dff282d1b7993aeb328c02a7a030e");
      //broadcast(tx);
 }

TEST_F(ED25519TezosMakeTransaction, ParseUnsignedRawTransaction) {
    auto strTx = "036e766ee0733ef0fb6385f2034cfbd437247afad4b301ebce1b929a67ce4a0b8d6c00902c5d86590a2452f0ccf9c1fa55ae679de27d398e0aee94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD");
    EXPECT_EQ(tx->getValue()->toLong(), 5000L);
    EXPECT_EQ(tx->getFees()->toLong(), 1294);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10407);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(ED25519TezosMakeTransaction, ParseUnsignedRawTransactionWithReveal) {
    auto strTx = "03ade89a7e5460a33ebabcab7cb7b6589cd7e48b512ebdc57e0acfc439934b04096b00902c5d86590a2452f0ccf9c1fa55ae679de27d39d109ec94cb03d84f0000fc4e4f37ded512650ca89dffe6c622a96a20f375d56331174c17c7b25f3775c76c00902c5d86590a2452f0ccf9c1fa55ae679de27d39de09ed94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    //tx->setRawTx(std::vector<unsigned char>{});
    //EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD");
    EXPECT_EQ(tx->getValue()->toLong(), 5000L);
    EXPECT_EQ(tx->toReveal(), true);
    EXPECT_EQ(tx->getFees()->toLong(), 1246+1233);//sum of transaction fees and reveal fees
    EXPECT_EQ(tx->getGasLimit()->toLong(), 10407+10200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(ED25519TezosMakeTransaction, CreateDelegation) {
        /*
{'protocol': 'PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo',
 'branch': 'BMSEkxVJJCrCgUwrVDw9B3dTyTdDSafoQSYoiCpZjaJvhQJMjGi',
 'contents': [{'kind': 'delegation',
   'source': 'tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF',
   'fee': '370',
   'counter': '7522926',
   'gas_limit': '1200',
   'storage_limit': '0',
   'delegate': 'tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM'}],
 'signature': 'sigghzYG43eD65EyNseXSnwsJvwrp4e8QviQqoqdjRi3uSUbYhherjUGTVaPuiwRVk6KRDoXmdpj4B6Fq72PhJzvuQND18NP'}

 e2f61b0fdb79853d4488bc3a7a4bd7c7f6f102687b4ba096967c4a1729e17f596e00902c5d86590a2452f0ccf9c1fa55ae679de27d39f202ee94cb03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df8f1477526afc31f10046eb4c31d2bd0e2b028ef62d05a8d4e11c83b9edb41d1a21bde6fdd176bcc73d2df97a8832842138faa016072ae1acccccad28b248a50b
    */
    
     const std::string DESTINATION = "tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM"; 
     
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

      builder->setFees(api::Amount::fromLong(currency, 370));
      builder->setGasLimit(api::Amount::fromLong(currency, 1200));
      builder->setStorageLimit(std::make_shared<api::BigIntImpl>(BigInt::fromString("0")));
      builder->wipeToAddress(DESTINATION);

      builder->setType(api::TezosOperationTag::OPERATION_TAG_DELEGATION);
      auto f = builder->build();
      auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(::wait(f));
      tx->setBlockHash("BMSEkxVJJCrCgUwrVDw9B3dTyTdDSafoQSYoiCpZjaJvhQJMjGi");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7522926")));
      tx->setSignature(hex::toByteArray("8f1477526afc31f10046eb4c31d2bd0e2b028ef62d05a8d4e11c83b9edb41d1a21bde6fdd176bcc73d2df97a8832842138faa016072ae1acccccad28b248a50b"));
      tx->reveal(false);

      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateDelegation - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload), "e2f61b0fdb79853d4488bc3a7a4bd7c7f6f102687b4ba096967c4a1729e17f596e00902c5d86590a2452f0ccf9c1fa55ae679de27d39f202ee94cb03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df8f1477526afc31f10046eb4c31d2bd0e2b028ef62d05a8d4e11c83b9edb41d1a21bde6fdd176bcc73d2df97a8832842138faa016072ae1acccccad28b248a50b");
      //broadcast(tx);    
 }

TEST_F(ED25519TezosMakeTransaction, ParseUnsignedRawDelegation) {
    auto strTx = "03e2f61b0fdb79853d4488bc3a7a4bd7c7f6f102687b4ba096967c4a1729e17f596e00902c5d86590a2452f0ccf9c1fa55ae679de27d39f202ee94cb03b00900ff00a239f27133bef5fb66577f05f2f62c59423ab1df";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1aRoaRhSpRYvFdyvgWLL6TGyRoGF51wDjM");
    EXPECT_EQ(tx->getFees()->toLong(), 370);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 1200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(ED25519TezosMakeTransaction, ParseUnsignedRawUndelegation) {
    auto strTx = "03e2f61b0fdb79853d4488bc3a7a4bd7c7f6f102687b4ba096967c4a1729e17f596e00902c5d86590a2452f0ccf9c1fa55ae679de27d39f202ee94cb03b0090000";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_EQ(hex::toString(tx->serialize()), strTx);

    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1YnM9JMYof5yKXzs3XptoJ1RMf1Ri7RuGF");
    EXPECT_EQ(tx->getReceiver(), nullptr);
    EXPECT_EQ(tx->getFees()->toLong(), 370);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 1200);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
}

TEST_F(ED25519TezosMakeTransaction, GetCurrentDelegationOnNotDelegatedAccount) {
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

    auto delegate = ::wait(account->getCurrentDelegate());
    EXPECT_EQ(delegate, "");
}

TEST_F(ED25519TezosMakeTransaction, ParseUnsignedRawDelegationWithReveal) {
    auto strTx = "031c0f4f48604e2559c38cd496c636b0e70d66ec05fef347740f6f53e58dab4abb6b0001389e444918998aea268d84bff822ec3bcf9c2acd02fb8d8504b00900000fa8aaea9a132dca88385949b28cf9dd406a788aee0e6267d8d237b30cc18fdd6e0001389e444918998aea268d84bff822ec3bcf9c2ac202fc8d8504b00900ff002a652136b35b53f8859dad9d0f16c88f8045a6bd";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    
    // ensure the values are correct
    EXPECT_EQ(tx->getType(), api::TezosOperationTag::OPERATION_TAG_DELEGATION);
    EXPECT_EQ(tx->getSender()->toBase58(), "tz1KkVCDQ5CKLj4pfTwaGRzuMFwoPBut498t");
    EXPECT_EQ(tx->getReceiver()->toBase58(), "tz1PWCDnz783NNGGQjEFFsHtrcK5yBW4E2rm");
    EXPECT_EQ(tx->getFees()->toLong(), 655);
    EXPECT_EQ(tx->getGasLimit()->toLong(), 2400);
    EXPECT_EQ(tx->getStorageLimit()->toString(10), "0");
    EXPECT_EQ(tx->toReveal(), true);
}
