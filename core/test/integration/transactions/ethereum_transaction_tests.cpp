/*
 *
 * ethereum_transaction_tests
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include "../BaseFixture.h"
#include "../../fixtures/eth_xpub_fixtures.h"

#include <api/KeychainEngines.hpp>
#include "transaction_test_helper.h"

#include <utils/hex.h>

#include <iostream>
using namespace std;

struct EthereumMakeTransaction : public EthereumMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName = "my_wallet";
        testData.currencyName = "ethereum";
        testData.inflate_eth = ledger::testing::eth_xpub::inflate;
    }
};

TEST_F(EthereumMakeTransaction, CreateStandardWithOneOutput) {
    auto builder = tx_builder();
    builder->setGasPrice(api::Amount::fromLong(currency, 200000));
    builder->setGasLimit(api::Amount::fromLong(currency, 20000000));
    builder->sendToAddress(api::Amount::fromLong(currency, 200000), "0xE8F7Dc1A12F180d49c80D1c3DbEff48ee38bD1DA");
    builder->setInputData({0x00});
    auto f = builder->build();
    auto tx = ::wait(f);
    auto serializedTx = tx->serialize();
    auto parsedTx = EthereumLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(), serializedTx);
    auto serializedParsedTx = parsedTx->serialize();
    EXPECT_EQ(serializedTx, serializedParsedTx);
}
