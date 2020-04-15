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
#include <math/bech32/Bech32Factory.h>
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
    auto hrp = Bech32Factory::newBech32Instance("btc").getValue()->getBech32Params().hrp;
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

TEST_F(BitcoinMakeP2WSHTransaction, ParseSignedTx) {
    auto hash = "94236be7808bc824ae3c531ee4cdf26559d6cf40cb6541f38153c54701fb0ea7";
    auto sender = "bc1qsqe7gwppjngklwjd2lp8kde0cpglerldadudcua3efr7a0tf3ucs995hxa";
    std::vector<std::string> receivers {"bc1qmxalhet27lzt07tq5uxhagg8z4538k095f4s5u2znh67p972v5mswsecmn", "bc1qhfrga6jkrvnrq8jv7606xj77fttqeq08puze7pzu4xnejvgqffxs96j3cx"};

    auto signedTx = "0100000000010180e68831516392fcd100d186b3c2c7b95c80b53c77e77c35ba03a66b429a2a1b0000000000ffffffff028096980000000000220020d9bbfbe56af7c4b7f960a70d7ea107156913d9e5a26b0a71429df5e097ca65378096980000000000220020ba468eea561b26301e4cf69fa34bde4ad60c81e70f059f045ca9a79931004a4d024730440220032521802a76ad7bf74d0e2c218b72cf0cbc867066e2e53db905ba37f130397e02207709e2188ed7f08f4c952d9d13986da504502b8c3be59617e043552f506c46ff83275163ab68210392972e2eb617b2388771abe27235fd5ac44af8e61693261550447a4c3e39da98ac00000000";
    auto parsedTx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(signedTx), 0);
    EXPECT_EQ(signedTx, hex::toString(parsedTx->serialize()));
    EXPECT_EQ(parsedTx->getHash(), hash);
    EXPECT_GT(parsedTx->getInputs().size(), 0);
    EXPECT_EQ(parsedTx->getInputs()[0]->getAddress().value_or(""), sender);
    EXPECT_GT(parsedTx->getOutputs().size(), 0);
    EXPECT_EQ(parsedTx->getOutputs()[0]->getAddress().value_or(""), receivers[0]);
    EXPECT_EQ(parsedTx->getOutputs()[1]->getAddress().value_or(""), receivers[1]);
}
