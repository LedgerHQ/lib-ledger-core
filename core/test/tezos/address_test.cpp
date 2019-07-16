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

using namespace ledger::core::api;
using namespace ledger::core;

const Currency currency = currencies::TEZOS;
TEST(TezosAddress, AddressFromEd25519PubKey) {
    // Decode a pubKey prefixed with edpk
    // 451bde832454ba73e6e0de313fcf5d1565ec51080edc73bb19287b8e0ab2122b
    auto expectedResult = "tz1dtmCcU7Ng29oWrEc5xJcrQfMnKgoqT7mn";
    auto xpub = "edpkuAfEJCEatRgFpRGg3gn3FdWniLXBoubARreRwuVZPWufkgDBvR";
    auto zPub = ledger::core::TezosLikeExtendedPublicKey::fromBase58(currency, xpub, Option<std::string>("44'/1729'/0'/0'"));
    EXPECT_EQ(zPub->derive("")->toBase58(), expectedResult);
}

TEST(TezosAddress, AddressFromEd25519PublicKey) {
    // Decode a pubKey prefixed with edpk
    auto xpub = "edpkuySiX9Qi89G5aRaynPxLMqtrrjsMGZAGCUn7u2kBgYH5uxCwEy";
    auto zPub = ledger::core::TezosLikeExtendedPublicKey::fromBase58(currency, xpub, Option<std::string>("44'/1729'/0'/0'"));
    EXPECT_EQ(zPub->derive("")->toBase58(), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");
}

TEST(TezosAddress, AddressFromSecp256k1PubKey) {
    std::vector<uint8_t> pubKey = hex::toByteArray("02af5696511e23b9e3dc5a527abc6929fae708defb5299f96cfa7dd9f936fe747d");
    std::vector<uint8_t> chainCode = hex::toByteArray("");
    auto zPub = ledger::core::TezosLikeExtendedPublicKey::fromRaw(currency,
                                                                  optional<std::vector<uint8_t >>(),
                                                                  pubKey,
                                                                  chainCode,
                                                                  "44'/1729'/0'/0'",
                                                                  api::TezosCurve::SECP256K1);
    EXPECT_EQ(zPub->derive("")->toBase58(), "tz1cmN7N6rV9ULVqbL2BxSUZgeL5wnWyoBUE");
}

TEST(TezosAddress, XpubFromBase58String) {
    auto addr = "xpub6DECL9NX5hvkRrq98MRvXCjTP8s84NZUFReEVLizQMVH8epXyoMvncB9DG4d8kY94XPVYqFWtUVaaagZkXvje4AUF3qdd71fJc8KyhRRC8V";
    auto xpub = ledger::core::TezosLikeExtendedPublicKey::fromBase58(currency, addr,
                                                                     optional<std::string>("44'/1729'/0'"));
    EXPECT_EQ(xpub->toBase58(), addr);
    EXPECT_EQ(xpub->derive("0/0")->toBase58(), "tz1hQ2ZpmNfiUuRf28yUJqNteVwX7fXWbmvk");
    EXPECT_EQ(xpub->derive("0/1")->toBase58(), "tz1hQ2ZpmNfiUuRf28yUJqNteVwX7fXWbmvk");
    EXPECT_EQ(xpub->derive("0/2")->toBase58(), "tz1hQ2ZpmNfiUuRf28yUJqNteVwX7fXWbmvk");
}


TEST(TezosAddress, ParseAddressFromString) {
    auto address = "tz1fmeh1DSskufrsi4qWxsfGPyhVdNtXNu78";
    auto tezosAddress = ledger::core::TezosLikeAddress::parse(address, currency, Option<std::string>("0/0"));
    EXPECT_EQ(address, tezosAddress->toString());
}
