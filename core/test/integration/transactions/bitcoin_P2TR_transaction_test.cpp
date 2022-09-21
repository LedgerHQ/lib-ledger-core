/*
 *
 * bitcoin_P2TR_transaction_tests
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ledger
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
#include "api/ErrorCode.hpp"
#include "transaction_test_helper.h"

#include <api/KeychainEngines.hpp>
#include <math/bech32/Bech32Factory.h>
#include <utils/hex.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeTransactionApi.h>

using namespace std;

struct BitcoinTestNetMakeP2TRTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
        testData.configuration->putBoolean(api::Configuration::ALLOW_P2TR, true);
        // https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName   = randomWalletName();
        testData.currencyName = "bitcoin_testnet";
        ledger::core::api::ExtendedKeyAccountCreationInfo TPUB_INFO(
            0, {"main"},
            {"84'/1'/0'"},
            {"tpubDDbUemsTt1GGoTRzXDa2wi3zdprJnynWUzidihCiw4r3wgumbpJR5Xzy892wtqgdkeHbZLh5EnJEDDdKBfE9CmQrwRu8guwrhBMHe53w1LF"});
        testData.inflate_btc = [TPUB_INFO](const std::shared_ptr<ledger::core::WalletPool> &pool,
                                           const std::shared_ptr<ledger::core::AbstractWallet> &wallet) {
            auto account = std::dynamic_pointer_cast<ledger::core::BitcoinLikeAccount>(
                uv::wait(wallet->newAccountWithExtendedKeyInfo(TPUB_INFO)));
            const std::string TX_1 = "{\"hash\": \"417ff5706cf061d269f76635c7b30764d273bb1bff8daa3c7ff644c86c6ad56b\", \"received_at\": \"2022-01-26T15:34:10Z\", \"lock_time\": 2137675, \"fees\": 141, \"inputs\": [{\"output_hash\": \"2227ab854df67321b2df60a1af7359463398617d207c53ed2e42f9fe67c7177d\", \"output_index\": 0, \"input_index\": 0, \"value\": 2269417122, \"address\": \"tb1q438hxz0qvp5xwh9zux6fm5yuel9p7ckcxcm5mz\", \"script_signature\": \"\", \"txinwitness\":[\"304402200402edd3b96ef45c33b74f03b4663a15510887db359df618c239379aedfff9aa02205081b34c72c94fbf7c8b4b95d76c34d48605635ea938b557ce5f403cadabacb701\", \"02b50aeecd3f0beb47509ef1e51cc111970212979f63a1c8e7949202af128ada36\"], \"sequence\": 4294967294}], \"outputs\": [{\"output_index\": 0, \"value\": 10000, \"address\": \"tb1qtxs4uqwtgaxshgcaajpz3kl0lcxvph0rh8ymz6\", \"script_hex\": \"001459a15e01cb474d0ba31dec8228dbeffe0cc0dde3\"}, {\"output_index\": 1, \"value\": 2269406981, \"address\": \"tb1qa2cqctwg3efsa40hnrxz6ng95k929juhx60sl8\", \"script_hex\": \"0014eab00c2dc88e530ed5f798cc2d4d05a58aa2cb97\"}], \"block\": {\"hash\": \"0000000000000050e5ceaa95dbd83b74681c4c8faba72b8ec0efb6cbc17a563a\", \"height\": 2137676, \"time\": \"2022-01-26T15:34:10Z\"}, \"confirmations\": 87}";
            std::vector<ledger::core::Operation> operations;
            const auto parsedTx = ledger::core::JSONUtils::parse<ledger::core::TransactionParser>(TX_1);
            account->interpretTransaction(*parsedTx, operations, true);
            account->bulkInsert(operations);
            return account;
        };
    }
};

TEST_F(BitcoinTestNetMakeP2TRTransaction, CreateP2TRWithOneOutput) {
    std::vector<InputDescr> input_descrs = {
        {"417ff5706cf061d269f76635c7b30764d273bb1bff8daa3c7ff644c86c6ad56b",
         0,
         std::make_shared<api::BigIntImpl>(BigInt(10000))}};
    std::vector<OutputDescr> output_descrs = {
        {"tb1p2ruhtexffskh4r4kwdrza6xvpaga0aq9jjlt2gflkn4geymj697qlkqyqu",
         hex::toByteArray("512050f975e4c94c2d7a8eb673462ee8cc0f51d7f40594beb5213fb4ea8c9372d17c"),
         std::make_shared<api::BigIntImpl>(BigInt(100))},
        {"", // This is a change output. It isn't used for tx building.
         hex::toByteArray("00147ab496acdfcc422f9486ccc7a66ddc4df2049811"),
         std::make_shared<api::BigIntImpl>(BigInt(567))}};

    createAndVerifyTransaction(input_descrs, output_descrs);
}

struct BitcoinMainNetMakeP2TRTransaction : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
        testData.configuration->putBoolean(api::Configuration::ALLOW_P2TR, true);
        // https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName   = randomWalletName();
        testData.currencyName = "bitcoin";
        testData.inflate_btc  = ledger::testing::txes_to_wpkh::inflate;
    }
};

TEST_F(BitcoinMainNetMakeP2TRTransaction, CreateP2TRWithOneOutput) {
    std::vector<InputDescr> input_descrs = {
        {"673f7e1155dd2cf61c961cedd24608274c0f20cfaeaa1154c2b5ef94ec7b81d1",
         1,
         std::make_shared<api::BigIntImpl>(BigInt(25402))}};
    std::vector<OutputDescr> output_descrs = {
        {"bc1psf4kpzmrk9aqszdwfmtpqhmvhuhq69frx5efq64yqudq08p3u5vsq8wc5y",
         hex::toByteArray("5120826b608b63b17a0809ae4ed6105f6cbf2e0d15233532906aa4071a079c31e519"),
         std::make_shared<api::BigIntImpl>(BigInt(100))},
        {"", // This is a change output. It isn't used for tx building.
         hex::toByteArray("00141017b1e1ca8632828f22a4d6c5260f3492b1dd08"),
         std::make_shared<api::BigIntImpl>(BigInt(15969))}};

    createAndVerifyTransaction(input_descrs, output_descrs);
}

struct BitcoinP2TRFeatureFlagTest : public BitcoinMakeBaseTransaction {
    void SetUpConfig() override {
        testData.configuration = DynamicObject::newInstance();
        testData.configuration->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP173_P2WPKH);
        // Don't set feature flag (false by default):
        // testData.configuration->putBoolean(api::Configuration::ALLOW_P2TR, true);

        // https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki
        testData.configuration->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, "84'/<coin_type>'/<account>'/<node>/<address>");
        testData.walletName   = randomWalletName();
        testData.currencyName = "bitcoin";
        testData.inflate_btc  = ledger::testing::txes_to_wpkh::inflate;
    }
};

TEST_F(BitcoinP2TRFeatureFlagTest, CreateP2TRWithOneOutputFails) {
    std::vector<InputDescr> input_descrs = {
        {"673f7e1155dd2cf61c961cedd24608274c0f20cfaeaa1154c2b5ef94ec7b81d1",
         1,
         std::make_shared<api::BigIntImpl>(BigInt(25402))}};
    std::vector<OutputDescr> output_descrs = {
        {"bc1psf4kpzmrk9aqszdwfmtpqhmvhuhq69frx5efq64yqudq08p3u5vsq8wc5y",
         hex::toByteArray("5120826b608b63b17a0809ae4ed6105f6cbf2e0d15233532906aa4071a079c31e519"),
         std::make_shared<api::BigIntImpl>(BigInt(100))},
        {"", // This is a change output. It isn't used for tx building.
         hex::toByteArray("00141017b1e1ca8632828f22a4d6c5260f3492b1dd08"),
         std::make_shared<api::BigIntImpl>(BigInt(16396))}};

    try {
        createAndVerifyTransaction(input_descrs, output_descrs);
    } catch (const Exception &e) {
        std::cerr << "An exception must be thrown here: " << e.what() << std::endl;
        EXPECT_EQ(e.getErrorCode(), ledger::core::api::ErrorCode::UNSUPPORTED_OPERATION);
    }
}
