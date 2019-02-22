/*
 *
 * deterministic_public_key_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/12/2016.
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

// Seed generated with "ghost rookie muffin"

#include <gtest/gtest.h>
#include <ledger/core/crypto/DeterministicPublicKey.hpp>
#include <ledger/core/math/Base58.hpp>
#include <ledger/core/bytes/BytesReader.h>
#include <ledger/core/utils/hex.h>

using namespace ledger::core;

static const std::string XPUB_1 = "xpub6EedcbfDs3pkzgqvoRxTW6P8NcCSaVbMQsb6xwCdEBzqZBronwY3Nte1Vjunza8f6eSMrYvbM5CMihGo6SbzpHxn4R5pvcr2ZbZ6wkDmgpy";

static DeterministicPublicKey createKeyFromXpub(const std::string& xpub) {
    auto networkIdentifier = "btc";
    auto raw = Base58::decode(xpub, networkIdentifier);
    BytesReader reader(raw);

    reader.readNextBeUint(); // READ MAGIC
    auto depth = reader.readNextByte();
    auto fingerprint = reader.readNextBeUint();
    auto childNum = reader.readNextBeUint();
    auto chainCode = reader.read(32);
    auto publicKey = reader.read(33);
    return DeterministicPublicKey(publicKey, chainCode, childNum, depth, fingerprint, networkIdentifier);
}

TEST(Derivation, DeriveChildren_1) {
    auto k = createKeyFromXpub(XPUB_1);
    EXPECT_EQ(k.derive(0).getPublicKey(), hex::toByteArray("034526331989014305eeaaced584dcdb395f8498db0d621845869ce00cedaedf74"));
    EXPECT_EQ(k.derive(1).getPublicKey(), hex::toByteArray("02c368bdec47a1b6faa76d624ead0cd2783234983c466767216ecdac8c472df3a6"));
    EXPECT_EQ(k.derive(2).getPublicKey(), hex::toByteArray("03c18f32f132f9974e4896d0da817c3f4657cfe943c51c1ddd2bd98d6600330b1f"));
    EXPECT_EQ(k.derive(3).getPublicKey(), hex::toByteArray("02998d8749dd8ed56dc4a2c746865020d45fa1b2f75766a73da4cb9334adcd4cb9"));
    EXPECT_EQ(k.derive(4).getPublicKey(), hex::toByteArray("03fecae786e6a4542df1ae6b36694357dae2ba036eeb620fed5f6e8f1b445858d3"));
    EXPECT_EQ(k.derive(5).getPublicKey(), hex::toByteArray("03e185d94291ae80671c59ac522347a500d673b1302edd0c4eb6634cc850003034"));
}

static const std::string XPUB_2 = "xpub6DrvMc6me5H6sV3Wrva6thZyhxMZ7WMyB8nMWLe3T5xr79bBsDJn2zgSQiVWEbU5XfoLMEz7oZT9G49AoCcxYNrz2dVBrySzUw4k9GTNyoW";

TEST(Derivation, UncompressedPublicKey) {
    auto k = createKeyFromXpub(XPUB_2);
    EXPECT_EQ(k.derive(0).getUncompressedPublicKey(), hex::toByteArray("04a8c9ce67e978e3d83a6366f15f2304ce21851ae030d8430b178b77280c4ec2be21bb6c082fb47db9d8982d40f6594efa5487f199e07635bcd041b7b7cf9bcad7"));
}
