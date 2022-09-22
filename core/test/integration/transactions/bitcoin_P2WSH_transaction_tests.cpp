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
#include "../../fixtures/medium_xpub_fixtures.h"
#include "../../fixtures/txes_to_wpkh_fixtures.h"
#include "../BaseFixture.h"
#include "transaction_test_helper.h"

#include <api/KeychainEngines.hpp>
#include <math/bech32/Bech32Factory.h>
#include <utils/hex.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>

using namespace std;

struct BitcoinMakeTransactionFromNativeSegwitToP2WSH : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
        // https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName   = randomWalletName();
        testData.currencyName = "bitcoin";
        testData.inflate_btc  = ledger::testing::txes_to_wpkh::inflate;
    }
};

TEST_F(BitcoinMakeTransactionFromNativeSegwitToP2WSH, CreateStandardP2WSHWithOneOutput) {
    auto address = "bc1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3qccfmv3";

    auto balance = uv::wait(account->getBalance());
    std::cerr << balance->toLong() << std::endl;

    std::vector<InputDescr> input_descrs = {
        {"673f7e1155dd2cf61c961cedd24608274c0f20cfaeaa1154c2b5ef94ec7b81d1",
         1,
         std::make_shared<api::BigIntImpl>(BigInt(25402))}};
    std::vector<OutputDescr> output_descrs = {
        {address,
         hex::toByteArray("00201863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"),
         std::make_shared<api::BigIntImpl>(BigInt(100))},
        {"", // This is a change output. Don't use it explicitely in building
         hex::toByteArray("00141017b1e1ca8632828f22a4d6c5260f3492b1dd08"),
         // yes, change goes to legacy address
         std::make_shared<api::BigIntImpl>(BigInt(15969))}};

    std::shared_ptr<api::BitcoinLikeTransaction> generatedTx = createTransaction(output_descrs);

    std::cerr << hex::toString(generatedTx->serialize()) << std::endl;

    EXPECT_TRUE(verifyTransaction(generatedTx, input_descrs, output_descrs));

    std::vector<uint8_t> tx_bin = generatedTx->serialize();
    std::cerr << hex::toString(generatedTx->serialize()) << std::endl;

    auto parsedTx = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(),
                                                                               tx_bin, 0);

    EXPECT_TRUE(verifyTransactionOutputs(parsedTx, output_descrs));
    // Values in inputs are missing after parsing. Here we can test only outputs.

    EXPECT_EQ(tx_bin, parsedTx->serialize());
}

struct BitcoinMakeTransactionFromLegacyToP2WSH : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.walletName    = randomWalletName();
        testData.currencyName  = "bitcoin";
        testData.inflate_btc   = ledger::testing::medium_xpub::inflate;
    }
};

TEST_F(BitcoinMakeTransactionFromLegacyToP2WSH, CreateStandardP2WSHWithOneOutput) {
    auto address = "bc1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3qccfmv3";

    auto balance = uv::wait(account->getBalance());
    std::cerr << "Balance before test: " << balance->toLong() << std::endl;

    std::vector<InputDescr> input_descrs = {
        {"a207285f69f5966f47c93ea0b76c1d751912823ed5f58ad23d8e5600260f39f6",
         0,
         std::make_shared<api::BigIntImpl>(BigInt(100000))}};
    std::vector<OutputDescr> output_descrs = {
        {address,
         hex::toByteArray("00201863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"),
         std::make_shared<api::BigIntImpl>(BigInt(100))},
        {"", // This is a change output. Don't use it explicitely in building
         hex::toByteArray("76a914d642b9c546d114dc634e65f72283e3458032a3d488ac"),
         // yes, change goes to legacy address
         std::make_shared<api::BigIntImpl>(BigInt(85565))}};

    std::shared_ptr<api::BitcoinLikeTransaction> generatedTx = createTransaction(output_descrs);

    std::cerr << "generated tx: " << std::endl
              << hex::toString(generatedTx->serialize()) << std::endl;

    EXPECT_TRUE(verifyTransaction(generatedTx, input_descrs, output_descrs));

    std::vector<uint8_t> tx_bin = generatedTx->serialize();

    auto parsedTx               = BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(wallet->getCurrency(),
                                                                                             tx_bin, 0);

    EXPECT_TRUE(verifyTransactionOutputs(parsedTx, output_descrs));
    // Values in inputs are missing after parsing. Here we can test only outputs.

    EXPECT_EQ(tx_bin, parsedTx->serialize());
}

TEST_F(BitcoinMakeTransactionFromLegacyToP2WSH, ParseSignedTx) {
    auto hash   = "94236be7808bc824ae3c531ee4cdf26559d6cf40cb6541f38153c54701fb0ea7";
    auto sender = "bc1qsqe7gwppjngklwjd2lp8kde0cpglerldadudcua3efr7a0tf3ucs995hxa";
    std::vector<std::string> receivers{"bc1qmxalhet27lzt07tq5uxhagg8z4538k095f4s5u2znh67p972v5mswsecmn", "bc1qhfrga6jkrvnrq8jv7606xj77fttqeq08puze7pzu4xnejvgqffxs96j3cx"};

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
