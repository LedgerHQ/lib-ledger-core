/*
 *
 * tezos_transaction_tests
 *
 * Created by Jeremy Coatelen on 08/02/2022.
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

#include "Fixtures.hpp"
#include "transaction_test.hpp"
#include "api/BlockchainExplorerEngines.hpp"
#include "wallet/tezos/explorers/api/TezosLikeTransactionParser.h"
#include "wallet/currencies.hpp"

/*

*/
using namespace ledger::testing::tezos;

struct TezosAccount : public TezosMakeBaseTransaction {
    void SetUpConfig() override {
        auto configuration = DynamicObject::newInstance();
        configuration->putString(api::TezosConfiguration::TEZOS_NODE, "https://xtz-node.api.live.ledger.com");
        configuration->putString(api::Configuration::BLOCKCHAIN_EXPLORER_ENGINE, api::BlockchainExplorerEngines::TZSTATS_API);
        configuration->putString(api::TezosConfiguration::TEZOS_XPUB_CURVE, api::TezosConfigurationDefaults::TEZOS_XPUB_CURVE_ED25519);
        configuration->putString(api::TezosConfiguration::TEZOS_PROTOCOL_UPDATE, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON);
        testData.configuration = configuration;
        testData.walletName = "TezosAccountTest-Wallet";
        testData.currencyName = "tezos";
        testData.inflate_xtz = inflate;
    }
};

const auto tx1 = R"({
  "row_id":17386565,
  "hash":"ooaZnj71WWrzG2by1mAX9sGXXKk46sAg19FR1tBcY7VSMEAm1wp",
  "type":"reveal",
  "block":"BLsiDcDBQnqPF2fZP178iK1zzcxd1RNwEjXAdP5D1CFuX3Gcugd",
  "time":"2019-11-06T13:21:37Z",
  "height":682242,
  "cycle":166,
  "counter":2263644,
  "op_l":3,
  "op_p":0,
  "op_c":0,
  "op_i":0,
  "status":"applied",
  "is_success":true,
  "is_contract":false,
  "gas_limit":18000,
  "gas_used":10000,
  "gas_price":0.25,
  "storage_limit":257,
  "storage_size":0,
  "storage_paid":0,
  "volume":0,
  "fee":0.0025,
  "has_data":true,
  "days_destroyed":0,
  "data":"edpkteNnvxz171eHjTdGmgg5MNVV9mu4JsE5zHZY7tWKKPobPyoJXq",
  "sender":"tz1Xeg8GzmCYhs5pWxwzerryDiQqFZWTJHzg",
  "is_batch":true,
  "confirmations":1330061
})";

const auto tx2 = R"({
  "row_id":17386566,
  "hash":"ooaZnj71WWrzG2by1mAX9sGXXKk46sAg19FR1tBcY7VSMEAm1wp",
  "type":"transaction",
  "block":"BLsiDcDBQnqPF2fZP178iK1zzcxd1RNwEjXAdP5D1CFuX3Gcugd",
  "time":"2019-11-06T13:21:37Z",
  "height":682242,
  "cycle":166,
  "counter":2263645,
  "op_l":3,
  "op_p":0,
  "op_c":1,
  "op_i":0,
  "status":"applied",
  "is_success":true,
  "is_contract":false,
  "gas_limit":18000,
  "gas_used":10207,
  "gas_price":0.24493,
  "storage_limit":257,
  "storage_size":0,
  "storage_paid":0,
  "volume":1.513242,
  "fee":0.0025,
  "burned":0.257,
  "days_destroyed":0.099832,
  "sender":"tz1Xeg8GzmCYhs5pWxwzerryDiQqFZWTJHzg",
  "receiver":"tz1VUgoKvoPsczqPAJe1Lge2JBM3QSuSnhi3",
  "is_batch":true,
  "confirmations":1330061
})";

TEST_F(TezosAccount, InterpetTransactionWithCorrectUidWhenOriginatedAccount) {
    std::vector<ledger::core::Operation> operations;

    const std::string& accAddress = account->getAccountAddress();

    // Test when sent from originated account
    {
      auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TezosLikeTransactionParser>(tx1);
      parsedTx->sender = accAddress;
      parsedTx->originatedAccountUid = "someUid";
      parsedTx->originatedAccountAddress = accAddress;
      account->interpretTransaction(*parsedTx, operations);
      EXPECT_EQ(operations.size(), 1);
      EXPECT_EQ(operations[0].uid, OperationDatabaseHelper::createUid(account->getAccountUid(), "2263644+0+OPERATION_TAG_REVEAL+someUid", api::OperationType::SEND));
    }

    // Test when received in originated account
    {
      auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TezosLikeTransactionParser>(tx2);
      parsedTx->receiver = accAddress;
      parsedTx->originatedAccountUid = "someUid";
      parsedTx->originatedAccountAddress = accAddress;
      account->interpretTransaction(*parsedTx, operations);
      EXPECT_EQ(operations.size(), 2);
      EXPECT_EQ(operations[1].uid, OperationDatabaseHelper::createUid(account->getAccountUid(), "2263645+1+OPERATION_TAG_TRANSACTION+someUid", api::OperationType::RECEIVE));
    }
}

TEST_F(TezosAccount, InterpetTransactionWithCorrectUidWithoutOriginatedAccount) {
    std::vector<ledger::core::Operation> operations;

    const std::string& accAddress = account->getAccountAddress();

    // Test when sent from account
    {
      auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TezosLikeTransactionParser>(tx1);
      parsedTx->sender = accAddress;
      account->interpretTransaction(*parsedTx, operations);
      EXPECT_EQ(operations.size(), 1);
      EXPECT_EQ(operations[0].uid, OperationDatabaseHelper::createUid(account->getAccountUid(), "2263644+0+OPERATION_TAG_REVEAL", api::OperationType::SEND));
    }

    // Test when received in account
    {
      auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TezosLikeTransactionParser>(tx2);
      parsedTx->receiver = accAddress;
      account->interpretTransaction(*parsedTx, operations);
      EXPECT_EQ(operations.size(), 2);
      EXPECT_EQ(operations[1].uid, OperationDatabaseHelper::createUid(account->getAccountUid(), "2263645+1+OPERATION_TAG_TRANSACTION", api::OperationType::RECEIVE));
    }
}


TEST_F(TezosAccount, ComputeOperationUidWithValidTransaction) {
    auto strTx = "036e766ee0733ef0fb6385f2034cfbd437247afad4b301ebce1b929a67ce4a0b8d6c00902c5d86590a2452f0ccf9c1fa55ae679de27d398e0aee94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    auto accAddress = ledger::core::TezosLikeAddress::fromBase58(account->getAccountAddress(), currency);
    
    // when sender
    {
      auto txTest = std::make_shared<TezosLikeTransactionApi>(*tx);
      txTest->setSender(accAddress);
      EXPECT_EQ(account->computeOperationUid(txTest), "7ce5bfdcb02056ffe84412ab15504cda6bd02ac38ce211826cfd1a9b6b236184");
    }

    // when receiver
    {
      auto txTest = std::make_shared<TezosLikeTransactionApi>(*tx);
      txTest->setReceiver(accAddress);
      EXPECT_EQ(account->computeOperationUid(txTest), "01f1f96874f5ea39ad4eec831d4291b249b95f2ebc4672884124ebc0f47a260e");
    }

}


TEST_F(TezosAccount, ComputeOperationUidWithInvalidTransaction) {
    auto strTx = "036e766ee0733ef0fb6385f2034cfbd437247afad4b301ebce1b929a67ce4a0b8d6c00902c5d86590a2452f0ccf9c1fa55ae679de27d398e0aee94cb03a75100882700011ebab3538f6ca4223ee98b565846e47d273d112900";
    auto txBytes = hex::toByteArray(strTx);
    auto tx = std::dynamic_pointer_cast<TezosLikeTransactionApi>(api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(
        ledger::core::currencies::TEZOS, txBytes, api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON));
    tx->setRawTx(std::vector<unsigned char>{});
    EXPECT_ANY_THROW(account->computeOperationUid(tx));
}