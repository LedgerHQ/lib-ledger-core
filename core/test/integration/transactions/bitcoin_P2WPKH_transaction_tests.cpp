/*
 *
 * bitcoin_P2WPKH_transaction_tests
 *
 * Created by El Khalil Bellakrid on 25/02/2019.
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

struct BitcoinMakeP2WPKHTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP173_P2WPKH);
        //https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME,"84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName = randomWalletName();
        testData.currencyName = "bitcoin";
        testData.inflate_btc = ledger::testing::medium_xpub::inflate;
    }
};

TEST_F(BitcoinMakeP2WPKHTransaction, CreateStandardP2WPKHWithOneOutput) {
    auto address = "bc1qshh6mmfq8fucahzxe4zc7pc5zdhk6zkt4uv8md";
    auto builder = tx_builder();
    auto freshAddress = uv::wait(account->getFreshPublicAddresses())[0];
    auto hrp = Bech32Factory::newBech32Instance("btc").getValue()->getBech32Params().hrp;
    auto freshAddressStr = freshAddress->asBitcoinLikeAddress()->toBech32();
    auto derivationPath = freshAddress->getDerivationPath().value_or("");
    EXPECT_EQ(derivationPath, "0/0");
    auto bechAddress = freshAddress->toString();
    EXPECT_EQ(bechAddress, address);
    EXPECT_EQ(freshAddressStr.substr(0, hrp.size()), hrp);
    EXPECT_EQ(freshAddressStr, address);
    auto balance = uv::wait(account->getBalance());
    // TODO: send BTC on address bc1qshh6mmfq8fucahzxe4zc7pc5zdhk6zkt4uv8md to implement the rest of tests ?
    EXPECT_EQ(balance->toLong(), 0);

    // TODO: In the meantime ...
    // Reference: https://github.com/bitcoin/bitcoin/blob/master/src/test/data/tx_valid.json
    auto mockTx = "0100000000010100010000000000000000000000000000000000000000000000000000000000000000000000ffffffff01e8030000000000001976a9144c9c3dfac4207d5d8cb89df5722cb3d712385e3f88ac02483045022100cfb07164b36ba64c1b1e8c7720a56ad64d96f6ef332d3d37f9cb3c96477dc44502200a464cd7a9cf94cd70f66ce4f4f0625ef650052c7afcfe29d7d7e01830ff91ed012103596d3451025c19dbbdeb932d6bf8bfb4ad499b95b6f88db8899efac102e5fc7100000000";
    auto parsedTx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(mockTx), 0);
    EXPECT_EQ(mockTx, hex::toString(parsedTx->serialize()));
}

TEST_F(BitcoinMakeP2WPKHTransaction, ParseSignedTx) {
    auto hash = "c3dd55c86d02ad9d4b0e748c219fd15b79f21c6d5e38f5fe84a453a7f9e37494";
    auto sender = "bc1qh4kl0a0a3d7su8udc2rn62f8w939prqpl34z86";
    std::vector<std::string> receivers {"bc1qh4kl0a0a3d7su8udc2rn62f8w939prqpl34z86", "bc1qry3crfssh8w6guajms7upclgqsfac4fs4g7nwj"};

    auto signedTx = "0100000000010154302828c224cb00c797038d4cbc9e06b5a38d832e879c67de523bd714a8c37c0000000000ffffff00021027000000000000160014bd6df7f5fd8b7d0e1f8dc2873d29277162508c01e82f010000000000160014192381a610b9dda473b2dc3dc0e3e80413dc553002483045022100dc57387b377550476a04f3147d915e57e396ab5ce41f8629f0aebd0f9a472876022025920a6a9d80aa6b31aedd10dfbd16d0b2eb8e449a93b70059e0cec7ac2a40ca012102fbba978d75f5fc4e7987840b78033e0e4797c7776c070037422616e622f8e6dc00000000";
    auto parsedTx = BitcoinLikeTransactionApi::parseRawSignedTransaction(wallet->getCurrency(), hex::toByteArray(signedTx), 0);
    EXPECT_EQ(signedTx, hex::toString(parsedTx->serialize()));
    EXPECT_EQ(parsedTx->getHash(), hash);
    EXPECT_GT(parsedTx->getInputs().size(), 0);
    EXPECT_EQ(parsedTx->getInputs()[0]->getAddress().value_or(""), sender);
    EXPECT_GT(parsedTx->getOutputs().size(), 0);
    EXPECT_EQ(parsedTx->getOutputs()[0]->getAddress().value_or(""), receivers[0]);
    EXPECT_EQ(parsedTx->getOutputs()[1]->getAddress().value_or(""), receivers[1]);
}
