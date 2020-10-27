/*
 *
 * tezos/P256_transaction_tests
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

struct P256TransactionTest : public TezosBaseTest {};

struct P256TezosMakeTransaction : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        std::cout << "SetUpConfig" << std::endl;
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_P256);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        testData.configuration = configuration;
        testData.walletName = "my_wallet";
        testData.currencyName = "tezos";
        testData.inflate_xtz = inflate_P256;
    }
};


  TEST_F(P256TezosMakeTransaction, CreateTx) {
    /*
  {'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BMdWmC7JYL5RnxPm72RR4xKvZa7h1WUKimNYU2uhugPjdnGWCMe',
 'contents': [{'kind': 'transaction',
   'source': 'tz3bnhbn7uYfL43zfXtBvCYoq6DW743mRWvc',
   'fee': '1294',
   'counter': '7523031',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '5000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigX29zyRk8Jjvir2GkximAGKgR7Gq8A2Mxuj7N4T6MSquiUZMNypLJmWRKW8uZDJ4oyKJyG3cEezZrcsB9QbZCSuWTvZu9r'}

Out[49]: 'fc90925a9641634831c1e7a5552270c4f7f954a974ec2596ac1fe3bfc14fc7cd6c02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c78e0ad795cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129004509a22f897460e621cc95b3b78b552c986c3cc477854a839eff0e458e8d0a50298d44486dd0733bf90f182667cb0e3735a31544c031fa8b6cce9b2150dc4376'

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
      tx->setBlockHash("BMdWmC7JYL5RnxPm72RR4xKvZa7h1WUKimNYU2uhugPjdnGWCMe");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7523031")));
      tx->setSignature(hex::toByteArray("4509a22f897460e621cc95b3b78b552c986c3cc477854a839eff0e458e8d0a50298d44486dd0733bf90f182667cb0e3735a31544c031fa8b6cce9b2150dc4376"));
      tx->reveal(false);
      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload),"fc90925a9641634831c1e7a5552270c4f7f954a974ec2596ac1fe3bfc14fc7cd6c02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c78e0ad795cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129004509a22f897460e621cc95b3b78b552c986c3cc477854a839eff0e458e8d0a50298d44486dd0733bf90f182667cb0e3735a31544c031fa8b6cce9b2150dc4376");
      //broadcast(tx);    
 }

  TEST_F(P256TezosMakeTransaction, CreateTxWithReveal) {
    /*  
{'protocol': 'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
 'branch': 'BKn22PBvdxDeQfXQ2tHKLN7D3NToAcN5pWDSmeSmNjThdFsuXNE',
 'contents': [{'kind': 'reveal',
   'source': 'tz3bnhbn7uYfL43zfXtBvCYoq6DW743mRWvc',
   'fee': '1234',
   'counter': '7523029',
   'gas_limit': '10200',
   'storage_limit': '0',
   'public_key': 'p2pk66ffLoWNNC1useG68cKfRqmfoujyYha9KuCAWb3RM6oth5KnR1Q'},
  {'kind': 'transaction',
   'source': 'tz3bnhbn7uYfL43zfXtBvCYoq6DW743mRWvc',
   'fee': '1246',
   'counter': '7523030',
   'gas_limit': '10407',
   'storage_limit': '0',
   'amount': '5000',
   'destination': 'tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD'}],
 'signature': 'sigaQzMFZdWLJb7JWBiP7jTS6pjeACXBs99MSgtjCnMkUotGLZWFJkVu91DjgwuxZiugn1w4ETu8t5kYWTBF8YTN52Uz7cki'}

Out[43]: '087bab85bf59327fe9c882c1346e47b7be4ed73c5afe229713ad8341131ed3eb6b02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c7d209d595cb03d84f00020313e0fe2062532d390a726eb7f78be03609796a930f6faba4349fdf1c963659736c02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c7de09d695cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129005efa423e1b34d92550b1a02d9928955206fe313f5d8ffb69a34b612b225ac2fcd402e5ac447e6b572874ab9061090ebd7245c7bfca7f0b7f98982f8faddb48bc'
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
      tx->setRevealFees(std::make_shared<BigInt>(BigInt::fromString("1234")));
      tx->setRevealGasLimit(std::make_shared<BigInt>(BigInt::fromString("10200")));
      tx->setBlockHash("BKn22PBvdxDeQfXQ2tHKLN7D3NToAcN5pWDSmeSmNjThdFsuXNE");
      tx->setCounter(std::make_shared<BigInt>(BigInt::fromString("7523029")));
      tx->setSignature(hex::toByteArray("5efa423e1b34d92550b1a02d9928955206fe313f5d8ffb69a34b612b225ac2fcd402e5ac447e6b572874ab9061090ebd7245c7bfca7f0b7f98982f8faddb48bc"));
      tx->reveal(true);
      auto binaryPayload= tx->serialize();
      std::cout << "TezosMakeTransaction.CreateTx - serialized tx: " << hex::toString(binaryPayload) << std::endl;

      EXPECT_EQ(hex::toString(binaryPayload), "087bab85bf59327fe9c882c1346e47b7be4ed73c5afe229713ad8341131ed3eb6b02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c7d209d595cb03d84f00020313e0fe2062532d390a726eb7f78be03609796a930f6faba4349fdf1c963659736c02a99468ec07aec188ee8d640bb7bf0a6f4bdf33c7de09d695cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d1129005efa423e1b34d92550b1a02d9928955206fe313f5d8ffb69a34b612b225ac2fcd402e5ac447e6b572874ab9061090ebd7245c7bfca7f0b7f98982f8faddb48bc");
     //broadcast(tx);
 }

