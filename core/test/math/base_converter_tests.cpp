/*
 *
 * base_converter_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/03/2019.
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
#include <math/BaseConverter.hpp>
#include <utils/hex.h>
#include <iostream>

using namespace ledger::core;

static std::vector<std::string> data {
    "FF11223344556677889900AABBCCFF00",
    "0011223344556677889900AABBCCFF00",
    "FF",
    "00000000",
    "",
    "666F6F",
    "666F6F62",
    "666F6F6261",
    "666F6F626172"

};

static std::vector<std::string> expected_base32rfc_no_padding_encoding = {
    "74ISEM2EKVTHPCEZACVLXTH7AA",
    "AAISEM2EKVTHPCEZACVLXTH7AA",
    "74",
    "AAAAAAA",
    "",
    "MZXW6",
    "MZXW6YQ",
    "MZXW6YTB",
    "MZXW6YTBOI"
};

static std::vector<std::string> expected_base32rfc_encoding = {
    "74ISEM2EKVTHPCEZACVLXTH7AA======",
    "AAISEM2EKVTHPCEZACVLXTH7AA======",
    "74======",
    "AAAAAAA=",
    "",
    "MZXW6===",
    "MZXW6YQ=",
    "MZXW6YTB",
    "MZXW6YTBOI======"
};

static std::vector<std::string> expected_base64_encoding = {
    "/xEiM0RVZneImQCqu8z/AA==",
    "ABEiM0RVZneImQCqu8z/AA==",
    "/w==",
    "AAAAAA==",
    "",
    "Zm9v",
    "Zm9vYg==",
    "Zm9vYmE=",
    "Zm9vYmFy"
};

TEST(BaseConverterTests, EncodeInBase32NoPadding) {
    auto index = 0;
    for (auto& d : data) {
        auto bytes = hex::toByteArray(d);
        auto base32Rfc = BaseConverter::encode(bytes, BaseConverter::BASE32_RFC4648_NO_PADDING);
        std::cout << "Base32: " << base32Rfc << std::endl;
        EXPECT_EQ(base32Rfc, expected_base32rfc_no_padding_encoding[index++]);
    }
}

TEST(BaseConverterTests, EncodeInBase32) {
    auto index = 0;
    for (auto& d : data) {
        auto bytes = hex::toByteArray(d);
        auto base32Rfc = BaseConverter::encode(bytes, BaseConverter::BASE32_RFC4648);
        std::cout << "Base32: " << base32Rfc << std::endl;
        EXPECT_EQ(base32Rfc, expected_base32rfc_encoding[index++]);
    }
}

TEST(BaseConverterTests, DecodeWithBase32NoPadding) {
    auto index = 0;
    for (auto& encoded : expected_base32rfc_no_padding_encoding) {
        std::vector<uint8_t> decoded;
        BaseConverter::decode(encoded, BaseConverter::BASE32_RFC4648_NO_PADDING, decoded);
        auto hexDecoded = hex::toString(decoded, true);
        std::cout << "Decoded: " << hexDecoded << std::endl;
        EXPECT_EQ(hexDecoded, data[index++]);
    }
}

TEST(BaseConverterTests, DecodeWithBase32) {
    auto index = 0;
    for (auto& encoded : expected_base32rfc_encoding) {
        std::vector<uint8_t> decoded;
        BaseConverter::decode(encoded, BaseConverter::BASE32_RFC4648, decoded);
        auto hexDecoded = hex::toString(decoded, true);
        std::cout << "Decoded: " << hexDecoded << std::endl;
        EXPECT_EQ(hexDecoded, data[index++]);
    }
}

TEST(BaseConverterTests, EncodeInBase64) {
    auto index = 0;
    for (auto& d : data) {
        auto bytes = hex::toByteArray(d);
        auto base64Rfc = BaseConverter::encode(bytes, BaseConverter::BASE64_RFC4648);
        std::cout << "Base64: " << base64Rfc << std::endl;
        EXPECT_EQ(base64Rfc, expected_base64_encoding[index++]);
    }
}

TEST(BaseConverterTests, DecodeWithBase64) {
    auto index = 0;
    for (auto& encoded : expected_base64_encoding) {
        std::vector<uint8_t> decoded;
        BaseConverter::decode(encoded, BaseConverter::BASE64_RFC4648, decoded);
        auto hexDecoded = hex::toString(decoded, true);
        std::cout << "Decoded: " << hexDecoded << std::endl;
        EXPECT_EQ(hexDecoded, data[index++]);
    }
}