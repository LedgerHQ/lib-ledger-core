/*
 *
 * digest_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/12/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include <core/crypto/SHA256.hpp>
#include <core/crypto/SHA512.hpp>
#include <core/crypto/RIPEMD160.hpp>
#include <core/utils/Hex.hpp>
#include <core/crypto/HMAC.hpp>
#include <core/crypto/HASH160.hpp>
#include <core/crypto/BLAKE.hpp>
#include <core/crypto/HashAlgorithm.hpp>
#include <core/crypto/Keccak.hpp>

using namespace ledger::core;

TEST(Digests, SHA256_Strings_to_String) {
    EXPECT_EQ(SHA256::stringToHexHash("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"), "2ff100b36c386c65a1afc462ad53e25479bec9498ed00aa5a04de584bc25301b");
}

// Tested with crypto module on NodeJS implementation (SFYL)
TEST(Digests, SHA512_Strings_to_String) {
    EXPECT_EQ(SHA512::stringToHexHash("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"), "90d1bdb9a6cbf9cb0d4a7f185ee0870456f440b81f13f514f4561a08112763523033245875b68209bb1f5d5215bac81e0d69f77374cc44d1be30f58c8b615141");
}

TEST(Digests, SHA256_bytes_to_String) {
    EXPECT_EQ(SHA256::bytesToHexHash({0x2f, 0xf1, 0x00, 0xb3, 0x6c, 0x38, 0x6c, 0x65, 0xa1, 0xaf, 0xc4, 0x62, 0xad, 0x53,
                                      0xe2, 0x54, 0x79, 0xbe, 0xc9, 0x49, 0x8e, 0xd0, 0x0a, 0xa5, 0xa0, 0x4d, 0xe5, 0x84,
                                      0xbc, 0x25, 0x30, 0x1b}),
              "c21efda7db95c7c642cee4095df44b19a23cc43f72ee5cae87cbd0f32230d2ee");
}

// Tested with crypto module on NodeJS implementation (SFYL)
TEST(Digests, SHA512_bytes_to_String) {
    EXPECT_EQ(SHA512::bytesToHexHash({0x2f, 0xf1, 0x00, 0xb3, 0x6c, 0x38, 0x6c, 0x65, 0xa1, 0xaf, 0xc4, 0x62, 0xad, 0x53,
                                      0xe2, 0x54, 0x79, 0xbe, 0xc9, 0x49, 0x8e, 0xd0, 0x0a, 0xa5, 0xa0, 0x4d, 0xe5, 0x84,
                                      0xbc, 0x25, 0x30, 0x1b}),
              "294b0184be47313f7258f4a01ecba3554ed9c8cdec4e52771e904708449c68fccc74964edb12ecfbd475af128a9d180bdd2489a76508278c7be4f4070684d1e9");
}

TEST(Digests, RIPEMD160) {
    std::vector<std::vector<std::string>> fixtures = {
            {"", "9c1185a5c5e9fc54612808977ee8f548b2258d31"},
            {"a", "0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"},
            {"abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"},
            {"message digest", 	"5d0689ef49d2fae572b881b123a85ffa21595f36"}
    };
    for (auto& i : fixtures) {
        EXPECT_EQ(RIPEMD160::hash(std::vector<uint8_t>(i[0].begin(), i[0].end())), hex::toByteArray(i[1]));
    }
}

TEST(Digest, HMACSHA256) {
    std::vector<std::vector<std::string>> fixtures = {
            {"0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", "4869205468657265", "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"},
            {"4a656665", "7768617420646f2079612077616e7420666f72206e6f7468696e673f", "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"},
            {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd", "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe"},
            {"0102030405060708090a0b0c0d0e0f10111213141516171819", "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd", "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b"}
    };

    for (auto& i : fixtures) {
        auto hash = hex::toString(HMAC::sha256(hex::toByteArray(i[0]), hex::toByteArray(i[1])));
        auto expected = i[2];
        EXPECT_EQ(hash, expected);
    }
}

TEST(Digest, HASH160) {
    auto pk = hex::toByteArray("02e73960f79f6637b773cf50f148cff54004669fd36045f8f9a36ffd669bcce71c");
    HashAlgorithm hashAlgorithm;
    auto hash160 = HASH160::hash(pk, hashAlgorithm);
    EXPECT_EQ(hex::toString(hash160), "253f5a6b1dd3f7d971807a5f3f2dcc9158002303");
}

TEST(Digest, BLAKE256) {

    auto input1 = hex::toByteArray("");
    auto output256 = BLAKE::blake256(input1);
    EXPECT_EQ(hex::toString(output256), "716f6e863f744b9ac22c97ec7b76ea5f5908bc5b2f67c61510bfc4751384ea7a");

    std::string input2 = "BLAKE";
    std::vector<uint8_t> vInput2(input2.begin(), input2.end());
    auto output2561 = BLAKE::blake256(vInput2);
    EXPECT_EQ(hex::toString(output2561), "07663e00cf96fbc136cf7b1ee099c95346ba3920893d18cc8851f22ee2e36aa6");

    std::string input3 = "The quick brown fox jumps over the lazy dog";
    std::vector<uint8_t> vInput3(input3.begin(), input3.end());
    auto output3 = BLAKE::blake256(vInput3);
    EXPECT_EQ(hex::toString(output3), "7576698ee9cad30173080678e5965916adbb11cb5245d386bf1ffda1cb26c9d7");

    std::string input4 = "Vires In Numeris";
    std::vector<uint8_t> vInput4(input4.begin(), input4.end());
    auto output4 = BLAKE::blake256(vInput4);
    EXPECT_EQ(hex::toString(output4), "9d8ee513f4c43a73c3dd7c6e7d389e62cca017358d880d16e4fae547ebac5717");
}

TEST(Digest, BLAKE2B) {
    auto input1 = hex::toByteArray("");
    auto output512 = BLAKE::blake2b(input1);
    EXPECT_EQ(hex::toString(output512), "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce");

    std::string input2 = "The quick brown fox jumps over the lazy dog";
    std::vector<uint8_t> vInput(input2.begin(), input2.end());
    auto output5121 = BLAKE::blake2b(vInput);
    EXPECT_EQ(hex::toString(output5121), "a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e3f14cb5413f8ed77be73045b13914cdcd6a918");
}
TEST(Digest, Keccak256) {

    auto empty = hex::toByteArray("");
    auto check = Keccak::keccak256(empty);
    EXPECT_EQ(hex::toString(check), "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470");

    auto pk = hex::toByteArray("6e145ccef1033dea239875dd00dfb4fee6e3348b84985c92f103444683bae07b83b5c38e5e2b0c8529d7fa3f64d46daa1ece2d9ac14cab9477d042c84c32ccd0");
    auto keccak = Keccak::keccak256(pk);
    EXPECT_EQ(hex::toString(keccak), "2a5bc342ed616b5ba5732269001d3f1ef827552ae1114027bd3ecf1f086ba0f9");

    EXPECT_EQ(hex::toString(Keccak::keccak256("Vires in numeris")), "d3f77a567fdf21bd226ddcebd5eb8df5f470c1bb77e307b6ffc1c80a24cf6495");

}
