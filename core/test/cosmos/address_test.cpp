/*
 *
 * address_test
 *
 * Created by El Khalil Bellakrid on 09/06/2019.
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

#include <gtest/gtest.h>

#include <api/Address.hpp>
#include <utils/hex.h>
#include <collections/vector.hpp>
#include <crypto/HashAlgorithm.h>
#include <crypto/HASH160.hpp>
#include <crypto/BLAKE.h>
#include <collections/DynamicObject.hpp>
#include <math/Base58.hpp>

#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/explorers/StargateGaiaCosmosLikeBlockchainExplorer.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/CosmosLikeAddress.hpp>
#include <api/CosmosCurve.hpp>

using namespace ledger::core::api;
using namespace ledger::core;


TEST(CosmosLikeBlockchainExplorer, FilterBuilder) {
    auto filter = StargateGaiaCosmosLikeBlockchainExplorer::fuseFilters(
        {StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeyAction,
             cosmos::constants::kEventTypeDelegate),
         StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
             cosmos::constants::kEventTypeMessage,
             cosmos::constants::kAttributeKeySender,
             "cosmostestaddress")});

    ASSERT_STREQ(filter.c_str(), "message.action=delegate&message.sender=cosmostestaddress" );

    filter = StargateGaiaCosmosLikeBlockchainExplorer::fuseFilters({
        StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            cosmos::constants::kEventTypeTransfer,
            cosmos::constants::kAttributeKeyRecipient,
            "cosmosvalopertestaddress")});


    ASSERT_STREQ(filter.c_str(), "transfer.recipient=cosmosvalopertestaddress");
}

TEST(CosmosAddress, AddressFromPubKey) {
    {
        // Results returned by device
        std::string prefixedPubKey = "cosmospub1addwnpepqdtwj8njf68zedmfhzru54tg2475nnfjrrgtfd533prvs7sljk7nzxvtkpd";
        std::string expectedBech32Addr = "cosmos16xkkyj97z7r83sx45xwk9uwq0mj0zszlf6c6mq";

        // From bech32 pubKey to pubKeyHash160
        auto pkBech32 = std::make_shared<CosmosBech32>(api::CosmosBech32Type::PUBLIC_KEY);
        auto pkDecodedHash160 = pkBech32->decode(prefixedPubKey);

        // Byte array to encode : <PrefixBytes> <Size> <ByteArray> hence the + 5
        std::vector<uint8_t> secp256k1PubKey(pkDecodedHash160.second.begin() + 5, pkDecodedHash160.second.end());

        // Concat the <PrefixBytes> and <Size> again manually
        std::vector<uint8_t> prefixAndSize{0xEB, 0x5A, 0xE9, 0x87, 0x21};
        auto encoded = vector::concat(prefixAndSize, secp256k1PubKey);

        // Get publicKeyHash160
        HashAlgorithm hashAlgorithm("cosmos");
        auto publicKeyHash160 = HASH160::hash(secp256k1PubKey, hashAlgorithm);

        // Encode to bech32 address and check
        auto bech32 = std::make_shared<CosmosBech32>(api::CosmosBech32Type::ADDRESS);
        auto bech32AddrResult = bech32->encode(publicKeyHash160, std::vector<uint8_t>());
        EXPECT_EQ(bech32AddrResult, expectedBech32Addr);
    }

    {
        // From Secp256k1 33 bytes pubkey to address
        // https://github.com/tendermint/tendermint/blob/master/docs/spec/blockchain/encoding.md#public-key-cryptography
        std::string secpPubKey = "020BD40F225A57ED383B440CF073BC5539D0341F5767D2BF2D78406D00475A2EE9";
        auto pubKey = hex::toByteArray(secpPubKey);
        HashAlgorithm hashAlgorithm("cosmos");
        auto publicKeyHash160 = HASH160::hash(pubKey, hashAlgorithm);

        // To Bech32
        auto bech32 = std::make_shared<CosmosBech32>(api::CosmosBech32Type::ADDRESS);
        auto bech32Addr = bech32->encode(publicKeyHash160, std::vector<uint8_t>());
        auto decodedHash160 = bech32->decode(bech32Addr);
        EXPECT_EQ(hex::toString(decodedHash160.second), hex::toString(publicKeyHash160));
    }
}

// Reference: https://github.com/cosmos/cosmos-sdk/blob/master/crypto/ledger_test.go
TEST(CosmosAddress, AddressFromPubKeys) {
    std::vector<std::string> prefixedPubKeys {
            "cosmospub1addwnpepqd87l8xhcnrrtzxnkql7k55ph8fr9jarf4hn6udwukfprlalu8lgw0urza0",
            "cosmospub1addwnpepqfsdqjr68h7wjg5wacksmqaypasnra232fkgu5sxdlnlu8j22ztxvlqvd65",
            "cosmospub1addwnpepqw3xwqun6q43vtgw6p4qspq7srvxhcmvq4jrx5j5ma6xy3r7k6dtxmrkh3d",
            "cosmospub1addwnpepqvez9lrp09g8w7gkv42y4yr5p6826cu28ydrhrujv862yf4njmqyyjr4pjs",
            "cosmospub1addwnpepq06hw3enfrtmq8n67teytcmtnrgcr0yntmyt25kdukfjkerdc7lqg32rcz7",
            "cosmospub1addwnpepqg3trf2gd0s2940nckrxherwqhgmm6xd5h4pcnrh4x7y35h6yafmcpk5qns",
            "cosmospub1addwnpepqdm6rjpx6wsref8wjn7ym6ntejet430j4szpngfgc20caz83lu545vuv8hp",
            "cosmospub1addwnpepqvdhtjzy2wf44dm03jxsketxc07vzqwvt3vawqqtljgsr9s7jvydjmt66ew",
            "cosmospub1addwnpepqwystfpyxwcava7v3t7ndps5xzu6s553wxcxzmmnxevlzvwrlqpzz695nw9",
            "cosmospub1addwnpepqw970u6gjqkccg9u3rfj99857wupj2z9fqfzy2w7e5dd7xn7kzzgkgqch0r"
    };

    std::vector<std::string> bech32Addresses {
            "cosmos1w34k53py5v5xyluazqpq65agyajavep2rflq6h",
            "cosmos19ewxwemt6uahejvwf44u7dh6tq859tkyvarh2q",
            "cosmos1a07dzdjgjsntxpp75zg7cgatgq0udh3pcdcxm3",
            "cosmos1qvw52lmn9gpvem8welghrkc52m3zczyhlqjsl7",
            "cosmos17m78ka80fqkkw2c4ww0v4xm5nsu2drgrlm8mn2",
            "cosmos1ferh9ll9c452d2p8k2v7heq084guygkn43up9e",
            "cosmos10vf3sxmjg96rqq36axcphzfsl74dsntuehjlw5",
            "cosmos1cq83av8cmnar79h0rg7duh9gnr7wkh228a7fxg",
            "cosmos1dszhfrt226jy5rsre7e48vw9tgwe90uerfyefa",
            "cosmos1734d7qsylzrdt05muhqqtpd90j8mp4y6rzch8l"
    };

    int index = 0;
    for (auto &prefixedPubKey : prefixedPubKeys) {
        // From bech32 pubKey to pubKeyHash160
        auto pkBech32 = std::make_shared<CosmosBech32>(api::CosmosBech32Type::PUBLIC_KEY);
        auto pkDecodedHash160 = pkBech32->decode(prefixedPubKey);

        // Byte array to encode : <PrefixBytes> <Length> <ByteArray> hence the + 5
        std::vector<uint8_t> secp256k1PubKey(pkDecodedHash160.second.begin() + 5, pkDecodedHash160.second.end());

        std::vector<uint8_t> prefixAndSize{0xEB, 0x5A, 0xE9, 0x87, 0x21};
        auto encoded = vector::concat(prefixAndSize, secp256k1PubKey);

        // Get publicKeyHash160
        HashAlgorithm hashAlgorithm("cosmos");
        auto publicKeyHash160 = HASH160::hash(secp256k1PubKey, hashAlgorithm);

        // Encode to bech32
        auto bech32 = std::make_shared<CosmosBech32>(api::CosmosBech32Type::ADDRESS);
        auto bech32AddrResult = bech32->encode(publicKeyHash160, std::vector<uint8_t>());
        EXPECT_EQ(bech32AddrResult, bech32Addresses[index]);
        index++;
    }
}


TEST(CosmosAddress, CosmosAddressFromBech32PubKey) {
    auto expectedResult = "cosmos16xkkyj97z7r83sx45xwk9uwq0mj0zszlf6c6mq";
    auto pubKey = "cosmospub1addwnpepqdtwj8njf68zedmfhzru54tg2475nnfjrrgtfd533prvs7sljk7nzxvtkpd";
    auto pubKeyExt = ledger::core::CosmosLikeExtendedPublicKey::fromBech32(currencies::ATOM, pubKey, Option<std::string>("44'/118'/0'"));
    EXPECT_EQ(pubKeyExt->derive("")->toBech32(), expectedResult);
}

TEST(CosmosAddress, CosmosAddressFromBech32) {
    auto expectedResult = "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl";
    auto address = ledger::core::CosmosLikeAddress::fromBech32(expectedResult, currencies::ATOM, Option<std::string>("44'/118'/0'"));
    EXPECT_EQ(address->toBech32(), expectedResult);
}

TEST(CosmosAddress, CosmosValAddressFromBech32) {
    auto expectedResult = "cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7";
    auto address = ledger::core::CosmosLikeAddress::fromBech32(expectedResult, currencies::ATOM, Option<std::string>("44'/118'/0'"));
    EXPECT_EQ(address->toBech32(), expectedResult);
}

TEST(CosmosAddress, CosmosValConsAddressFromBase64ValConsPub) {
  // Test values come from manual testing on stargate-5
  // - A first call to /cosmos/staking/v1beta1/validators to get base64 keys
  // - Then a call to /cosmos/slashing/v1beta1/signing_infos/{address} to check that
  // the obtained address actually exists and does not 404
  auto b64Xpub = "pu8GYpo9Fa8lCs5pxkXwHKRhZWE+4JhG5TxYLyq1Lr4=";
  auto expectedResult = "cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4";
  auto pubkey = ledger::core::CosmosLikeExtendedPublicKey::fromBase64(
      currencies::ATOM, b64Xpub, {},
      api::CosmosCurve::ED25519, api::CosmosBech32Type::PUBLIC_KEY_VAL_CONS);
  EXPECT_EQ(pubkey->derive("")->toBech32(), expectedResult);

  auto otherB64Xpub = "B33RB8LHddbd+KXO1jPJHDpelfOgOGJP5yDEQYEWdyY=";
  auto otherExpectedResult = "cosmosvalcons14wfqgf2gx9hrn4e56jxte06dx67jwqmm2k789l";
  // The "path" when building these address should be irrelevant, adding a path in
  // this test case to make sure it doesn't break the test
  auto otherPubkey = ledger::core::CosmosLikeExtendedPublicKey::fromBase64(
      currencies::ATOM, otherB64Xpub, Option<std::string>("44'/118'/0'"),
      api::CosmosCurve::ED25519, api::CosmosBech32Type::PUBLIC_KEY_VAL_CONS);
  EXPECT_EQ(otherPubkey->derive("")->toBech32(), otherExpectedResult);
}

TEST(CosmosAddress, SecpPubbKey) {
    std::vector<uint8_t> pubKey = hex::toByteArray("02c4becf6843868d9556ea43d46518b51a13cb1a48cd6c05a21c029ea4231fcde4");
    std::vector<uint8_t> chainCode = hex::toByteArray("");
    auto zPub = ledger::core::CosmosLikeExtendedPublicKey::fromRaw(currencies::ATOM,
                                                                   optional<std::vector<uint8_t >>(),
                                                                   pubKey,
                                                                   chainCode,
                                                                   "44'/118'/0'/0'",
                                                                   api::CosmosCurve::SECP256K1);
    EXPECT_EQ(zPub->derive("")->toBech32(), "cosmos1x9fzdaykfcc3k4hvflzu4rc6683a7cgkqfhe0s");
}
