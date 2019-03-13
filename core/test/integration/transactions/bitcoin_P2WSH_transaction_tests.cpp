/*
 *
 * bitcoin_P2WSH_transaction_tests
 *
 * Created by El Khalil Bellakrid on 26/02/2019.
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
#include "../BaseFixture.h"
#include "../../fixtures/medium_xpub_fixtures.h"
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>
#include <bitcoin/bech32/Bech32Factory.h>
#include "transaction_test_helper.h"
#include <utils/hex.h>
#include <api/KeychainEngines.hpp>
using namespace std;

struct BitcoinMakeP2WSHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WSH);
        testData.walletName = "my_wallet";
        testData.currencyName = "bitcoin";
        testData.inflate_btc = ledger::testing::medium_xpub::inflate;
    }
};

TEST_F(BitcoinMakeP2WSHTransaction, CreateStandardP2WSHWithOneOutput) {
    auto builder = tx_builder();
    auto freshAddress = wait(account->getFreshPublicAddresses())[0];
    auto hrp = Bech32Factory::newBech32Instance("btc")->getBech32Params().hrp;
    auto freshAddressStr = freshAddress->asBitcoinLikeAddress()->toBech32();
    EXPECT_EQ(freshAddressStr.substr(0, hrp.size()), hrp);
    auto balance = wait(account->getBalance());
    // TODO: send BTC on address bc1q7kggee5ry2xpr0nul42grqul6ygyll5afj34xxehaftcc0pty4sqvud0h2 to implement the rest of tests ?
    EXPECT_EQ(balance->toLong(), 0);

    // TODO: In the meantime ...
    // Reference: https://github.com/bitcoin/bitcoin/blob/master/src/test/data/tx_valid.json
    auto mockTx = "0100000000010100010000000000000000000000000000000000000000000000000000000000000000000000ffffffff01e8030000000000001976a9144c9c3dfac4207d5d8cb89df5722cb3d712385e3f88ac02483045022100aa5d8aa40a90f23ce2c3d11bc845ca4a12acd99cbea37de6b9f6d86edebba8cb022022dedc2aa0a255f74d04c0b76ece2d7c691f9dd11a64a8ac49f62a99c3a05f9d01232103596d3451025c19dbbdeb932d6bf8bfb4ad499b95b6f88db8899efac102e5fc71ac00000000";
    auto parsedTx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(mockTx), 0);
    EXPECT_EQ(mockTx, hex::toString(parsedTx->serialize()));
}