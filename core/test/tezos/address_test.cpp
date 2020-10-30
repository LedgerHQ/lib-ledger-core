/*
 *
 * address_test
 *
 * Created by El Khalil Bellakrid on 25/04/2019.
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
#include <ledger/core/utils/hex.h>
#include <ledger/core/api/TezosLikeExtendedPublicKey.hpp>
#include <ledger/core/tezos/TezosLikeAddress.h>
#include <ledger/core/utils/optional.hpp>
#include <ledger/core/collections/DynamicObject.hpp>
#include <ledger/core/api/DynamicObject.hpp>
#include <ledger/core/api/Networks.hpp>
#include <wallet/currencies.hpp>
#include <api/Address.hpp>
#include <tezos/TezosLikeExtendedPublicKey.h>
#include <utils/hex.h>
#include <api/Configuration.hpp>
#include <crypto/HASH160.hpp>
#include <ledger/core/crypto/BLAKE.h>

#include "Fixtures.hpp"

using namespace ledger::core;
using namespace ledger::testing::tezos;

struct AddressTest : public TezosBaseTest {};

TEST_P(AddressTest, FromBase58PubKey) {
    TezosTestData data = GetParam();
    auto xpub = data.key.pubkey;
    auto zPub = ledger::core::TezosLikeExtendedPublicKey::fromBase58(currencies::TEZOS, xpub, Option<std::string>("44'/1729'/0'/0'"));
    EXPECT_EQ(zPub->toBase58(), xpub);
    EXPECT_EQ(zPub->derive("")->toBase58(), data.key.address);
}

TEST_P(AddressTest, FromPubKey) {
    TezosTestData data = GetParam();
    std::vector<uint8_t> pubKey = hex::toByteArray(data.key.hexkey);
    std::vector<uint8_t> chainCode = hex::toByteArray("");
    auto zPub = ledger::core::TezosLikeExtendedPublicKey::fromRaw(currencies::TEZOS,
                                                                  optional<std::vector<uint8_t >>(),
                                                                  pubKey,
                                                                  chainCode,
                                                                  "44'/1729'/0'/0'",
                                                                  data.curve);
    EXPECT_EQ(zPub->toBase58(), data.key.pubkey);
    EXPECT_EQ(zPub->derive("")->toBase58(), data.key.address);
}

TEST_P(AddressTest, ParseFromString) {
    TezosTestData data = GetParam();
    auto address = data.key.address;
    auto tezosAddress = ledger::core::TezosLikeAddress::parse(address, currencies::TEZOS, Option<std::string>("0/0"));
    EXPECT_EQ(address, tezosAddress->toString());
}

TEST_P(AddressTest, AddressValidation) {
    // This test is parametrized to ensure validation is not configuration related
    auto currency = currencies::TEZOS;

    // Test valid adresses
    auto address = KEY_ED25519.address;
    EXPECT_EQ(api::Address::isValid(address, currency), true);
    address = KEY_SECP256K1.address;
    EXPECT_EQ(api::Address::isValid(address, currency), true);
    address = KEY_P256.address;
    EXPECT_EQ(api::Address::isValid(address, currency), true);
    address = "KT1VsSxSXUkgw6zkBGgUuDXXuJs9ToPqkrCg";
    EXPECT_EQ(api::Address::isValid(address, currency), true);

    // Test invalid adresses
    // Valid prefix, invalid checksum
    address = "tz1eZwq8b5cvE2bPKokatLkVMzkxz24z3AAAA";
    EXPECT_EQ(api::Address::isValid(address, currency), false);
    // Invalid prefix, valid checksum
    address = "NmH7tmeJUmHcncBDvpr7aJNEBk7rp5zYsB1qt";
    EXPECT_EQ(api::Address::isValid(address, currency), false);
    // Invalid prefix, invalid checksum
    address = "1tzeZwq8b5cvE2bPKokatLkVMzkxz24zAAAAA";
    EXPECT_EQ(api::Address::isValid(address, currency), false);
    address = "tz1fmeh1DSskufrsi4qWxsfGPyhVdNtXNu";
    EXPECT_EQ(api::Address::isValid(address, currency), false);
    address = "qw";
    EXPECT_EQ(api::Address::isValid(address, currency), false);
}

INSTANTIATE_TEST_CASE_P(
    Tezos,
    AddressTest,
    TezosParams(),
    TezosParamsNames
);
