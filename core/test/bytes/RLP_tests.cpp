/*
 *
 * RLP_tests
 *
 * Created by El Khalil Bellakrid on 17/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <ledger/core/bytes/RLP/RLPListEncoder.h>
#include <ledger/core/bytes/RLP/RLPStringEncoder.h>
#include <ledger/core/bytes/RLP/RLPDecoder.h>

#include <ledger/core/bytes/BytesWriter.h>
#include <ledger/core/utils/hex.h>

#include <iostream>
using namespace std;

using namespace ledger::core;

//TODO: use toString method on encoders

TEST(RLPTests, SimpleCases) {
    auto rlpVector = std::make_shared<RLPListEncoder>();
    EXPECT_EQ(hex::toString(rlpVector->encode()), "c0");
    auto rlpString = std::make_shared<RLPStringEncoder>("");
    EXPECT_EQ(hex::toString(rlpString->encode()), "80");
    rlpString->append("Vires in numeris");
    EXPECT_EQ(hex::toString(rlpString->encode()), "90566972657320696e206e756d65726973");
    rlpVector->append(rlpString);
    EXPECT_EQ(hex::toString(rlpVector->encode()), "d190566972657320696e206e756d65726973");

    auto decodedString = RLPDecoder::decode(rlpString->encode());
    EXPECT_EQ(hex::toString(rlpString->encode()), hex::toString(decodedString->encode()));

    auto decodedVector = RLPDecoder::decode(rlpVector->encode());
    EXPECT_EQ(hex::toString(rlpVector->encode()), hex::toString(decodedVector->encode()));

}

TEST(RLPTests, RecursiveEmptyLists) {

    //[[]]
    auto rlpVector1 = std::make_shared<RLPListEncoder>();
    rlpVector1->append(std::make_shared<RLPListEncoder>());

    //[[]]
    auto rlpVector2 = std::make_shared<RLPListEncoder>();
    rlpVector2->append(std::make_shared<RLPListEncoder>());

    //[[], [[]]]
    auto rlpVector3 = std::make_shared<RLPListEncoder>();
    rlpVector3->append(std::make_shared<RLPListEncoder>());
    rlpVector3->append(rlpVector2);


    //[]
    auto rlpVector = std::make_shared<RLPListEncoder>();
    //[[]]
    rlpVector->append(std::make_shared<RLPListEncoder>());
    //[[], [[]]]
    rlpVector->append(rlpVector1);
    //[[], [[]], [[], [[]]]]
    rlpVector->append(rlpVector3);

    EXPECT_EQ(hex::toString(rlpVector->encode()), "c7c0c1c0c3c0c1c0");

    auto decodedVector = RLPDecoder::decode(rlpVector->encode());
    EXPECT_EQ(hex::toString(rlpVector->encode()), hex::toString(decodedVector->encode()));
}

TEST(RLPTests, SuccessiveEmptyLists) {

    //[[], [], []]
    auto rlpVector = std::make_shared<RLPListEncoder>();
    rlpVector->append(std::make_shared<RLPListEncoder>());
    rlpVector->append(std::make_shared<RLPListEncoder>());
    rlpVector->append(std::make_shared<RLPListEncoder>());

    EXPECT_EQ(hex::toString(rlpVector->encode()), "c3c0c0c0");

    auto decodedVector = RLPDecoder::decode(rlpVector->encode());
    EXPECT_EQ(hex::toString(rlpVector->encode()), hex::toString(decodedVector->encode()));
}

TEST(RLPTests, List) {
    //["Vires", "in", "numeris"]
    auto rlpVector = std::make_shared<RLPListEncoder>();
    rlpVector->append("Vires");
    rlpVector->append("in");
    rlpVector->append("numeris");
    EXPECT_EQ(hex::toString(rlpVector->encode()), "d185566972657382696e876e756d65726973");

    auto decodedVector = RLPDecoder::decode(rlpVector->encode());
    EXPECT_EQ(hex::toString(rlpVector->encode()), hex::toString(decodedVector->encode()));
}

TEST(RLPTests, RecursiveLists) {

    //["Vires"]
    auto viresList = std::make_shared<RLPListEncoder>();
    viresList->append("Vires");

    //[["in"]]
    auto rlpVector1 = std::make_shared<RLPListEncoder>();
    auto rlpVector11 = std::make_shared<RLPListEncoder>();
    rlpVector11->append("in");
    rlpVector1->append(rlpVector11);

    //[["numeris"]]
    auto rlpVector2 = std::make_shared<RLPListEncoder>();
    auto rlpVector21 = std::make_shared<RLPListEncoder>();
    rlpVector21->append("numeris");
    rlpVector2->append(rlpVector21);

    //[[], [["numeris"]]]
    auto rlpVector3 = std::make_shared<RLPListEncoder>();
    rlpVector3->append(std::make_shared<RLPListEncoder>());
    rlpVector3->append(rlpVector2);

    //[]
    auto rlpVector = std::make_shared<RLPListEncoder>();
    //[["Vires"]]
    rlpVector->append(viresList);
    //[["Vires"], [["in"]]]
    rlpVector->append(rlpVector1);
    //[["Vires"], [["in"]], [[], [["numeris"]]]]
    rlpVector->append(rlpVector3);

    EXPECT_EQ(hex::toString(rlpVector->encode()), "d8c6855669726573c4c382696ecbc0c9c8876e756d65726973");

    auto decodedVector = RLPDecoder::decode(rlpVector->encode());
    EXPECT_EQ(hex::toString(rlpVector->encode()), hex::toString(decodedVector->encode()));
}

TEST(RLPTests, Tx) {
    //f8473086323030303030883230303030303030aa3078453846374463314131324631383064343963383044316333446245666634386565333862443144418632303030303000010000
    std::string tx = "f8473086323030303030883230303030303030aa3078453846374463314131324631383064343963383044316333446245666634386565333862443144418632303030303000010000";
    auto decoder = RLPDecoder::decode(hex::toByteArray(tx));
    EXPECT_EQ(hex::toString(decoder->encode()), tx);
}


