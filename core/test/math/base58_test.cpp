/*
 *
 * base58_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#include <ledger/core/math/Base58.hpp>
#include <ledger/core/utils/hex.h>

using namespace ledger::core;

TEST(Base58, Encode) {
    const std::string BitcoinPublicKeyHashPrefix = "00";
    const std::string BitcoinPrivateKeyPrefix = "80";
    const std::string BitcoinExtendedPublicKeyPrefix = "0488B21E";
    const std::string BitcoinExtendedPrivateKeyPrefix = "0488ADE4";

    std::vector<std::vector<std::string>> fixtures = {
            {BitcoinPublicKeyHashPrefix, "010966776006953D5567439E5E39F86A0D273BEE", "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM"},
            {BitcoinPublicKeyHashPrefix, "7b2f2061d66d57ffb9502a091ce236ed4c1ede2d", "1CELa15H4DMzHtHnuz7LCpSFgFWf61Ra6A"},
            {BitcoinPublicKeyHashPrefix, "89C907892A9D4F37B78D5F83F2FD6E008C4F795D", "1DZYQ3xEy8mkc7wToQZvKqeLrSLUMVVK41"},
            {BitcoinPublicKeyHashPrefix, "0000000000000000000000000000000000000000", "1111111111111111111114oLvT2"},
            {BitcoinPublicKeyHashPrefix, "0000000000000000000000000000000000000001", "11111111111111111111BZbvjr"},
            {BitcoinPrivateKeyPrefix, "0C28FCA386C7A227600B2FE50B7CAE11EC86D3BF1FBE471BE89827E19D72AA1D", "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ"},
            {BitcoinPrivateKeyPrefix, "00000037FC2B523A9101D653ECB504EBB88FCCE6F7E77548A7B31FA734A00000", "5HpHagjigF1P3i1WyFp1uLPEo8gK32CFBRc2ekJU3nytmXnVbYv"},
            {BitcoinPrivateKeyPrefix, "3C80FA4C012E37402C6D43140EC7B111B931C33799C2A07E8172827B12EEA59F", "5JGw52y3UuZSwpZKYkuhmat8TNy1nZ6F6mbrHJaPNMt2pkETUVE"},
            {BitcoinExtendedPublicKeyPrefix, "000000000000000000873dff81c02f525623fd1fe5167eac3a55a049de3d314bb42ee227ffed37d5080339a36013301597daef41fbe593a02cc513d0b55527ec2df1050e2e8ff49c85c2", "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8"},
            {BitcoinExtendedPrivateKeyPrefix, "0478412e3afffffffe637807030d55d01f9a0cb3a7839515d796bd07706386a6eddf06cc29a65a0e2900f1c7c871a54a804afe328b4c83a1c33b8e5ff48f5087273f04efa83b247d6a2d", "xprvA1RpRA33e1JQ7ifknakTFpgNXPmW2YvmhqLQYMmrj4xJXXWYpDPS3xz7iAxn8L39njGVyuoseXzU6rcxFLJ8HFsTjSyQbLYnMpCqE2VbFWc"},
    };

    for (auto& item : fixtures) {
        EXPECT_EQ(Base58::encodeWithChecksum(hex::toByteArray(item[0] + item[1])),  [2]);
    }
}

TEST(Base58, Decode) {
    const std::string BitcoinPublicKeyHashPrefix = "00";
    const std::string BitcoinPrivateKeyPrefix = "80";
    const std::string BitcoinExtendedPublicKeyPrefix = "0488B21E";
    const std::string BitcoinExtendedPrivateKeyPrefix = "0488ADE4";

    std::vector<std::vector<std::string>> fixtures = {
            {BitcoinPublicKeyHashPrefix, "010966776006953D5567439E5E39F86A0D273BEE", "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM"},
            {BitcoinPublicKeyHashPrefix, "7b2f2061d66d57ffb9502a091ce236ed4c1ede2d", "1CELa15H4DMzHtHnuz7LCpSFgFWf61Ra6A"},
            {BitcoinPublicKeyHashPrefix, "89C907892A9D4F37B78D5F83F2FD6E008C4F795D", "1DZYQ3xEy8mkc7wToQZvKqeLrSLUMVVK41"},
            {BitcoinPublicKeyHashPrefix, "0000000000000000000000000000000000000000", "1111111111111111111114oLvT2"},
            {BitcoinPublicKeyHashPrefix, "0000000000000000000000000000000000000001", "11111111111111111111BZbvjr"},
            {BitcoinPrivateKeyPrefix, "0C28FCA386C7A227600B2FE50B7CAE11EC86D3BF1FBE471BE89827E19D72AA1D", "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ"},
            {BitcoinPrivateKeyPrefix, "00000037FC2B523A9101D653ECB504EBB88FCCE6F7E77548A7B31FA734A00000", "5HpHagjigF1P3i1WyFp1uLPEo8gK32CFBRc2ekJU3nytmXnVbYv"},
            {BitcoinPrivateKeyPrefix, "3C80FA4C012E37402C6D43140EC7B111B931C33799C2A07E8172827B12EEA59F", "5JGw52y3UuZSwpZKYkuhmat8TNy1nZ6F6mbrHJaPNMt2pkETUVE"},
            {BitcoinExtendedPublicKeyPrefix, "000000000000000000873dff81c02f525623fd1fe5167eac3a55a049de3d314bb42ee227ffed37d5080339a36013301597daef41fbe593a02cc513d0b55527ec2df1050e2e8ff49c85c2", "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8"},
            {BitcoinExtendedPrivateKeyPrefix, "0478412e3afffffffe637807030d55d01f9a0cb3a7839515d796bd07706386a6eddf06cc29a65a0e2900f1c7c871a54a804afe328b4c83a1c33b8e5ff48f5087273f04efa83b247d6a2d", "xprvA1RpRA33e1JQ7ifknakTFpgNXPmW2YvmhqLQYMmrj4xJXXWYpDPS3xz7iAxn8L39njGVyuoseXzU6rcxFLJ8HFsTjSyQbLYnMpCqE2VbFWc"},
    };

    for (auto& item : fixtures) {
        EXPECT_EQ(Base58::checkAndDecode(hex::toByteArray(item[0] + item[1])), [2]);
    }
}