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
    auto raw = Base58::decode(xpub);
    BytesReader reader(raw);

    reader.readNextBeUint(); // READ MAGIC
    auto depth = reader.readNextByte();
    auto fingerprint = reader.readNextBeUint();
    auto childNum = reader.readNextBeUint();
    auto chainCode = reader.read(32);
    auto publicKey = reader.read(33);
    return DeterministicPublicKey(publicKey, chainCode, childNum, depth, fingerprint);
}

TEST(Derivation, DeriveChildren_1) {
    auto k = createKeyFromXpub(XPUB_1);
    EXPECT_EQ(k.derive(0).getPublicKey(), hex::toByteArray("034526331989014305eeaaced584dcdb395f8498db0d621845869ce00cedaedf74"));
}
