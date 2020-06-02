/*
 *
 * get_memo_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "StellarFixture.hpp"
#include <wallet/common/OperationQuery.h>
#include <math/BigInt.h>
#include <wallet/stellar/StellarLikeMemo.hpp>
#include <api/StellarLikeMemoType.hpp>

using namespace ledger::core;

template <typename T>
inline void expectThrowInvalidMemoType(StellarLikeMemo* obj, T (StellarLikeMemo::*method)()) {
    auto run = [&] () {
        return (obj->*method)();
    };
    auto result = make_try<T>(run);
    EXPECT_TRUE(result.isFailure());
    EXPECT_EQ(result.getFailure().getErrorCode(), api::ErrorCode::INVALID_STELLAR_MEMO_TYPE);
}

TEST_F(StellarFixture, GetMemoText) {
    stellar::xdr::Memo memo;
    memo.type = stellar::xdr::MemoType::MEMO_TEXT;
    memo.content = "Hello world";

    StellarLikeMemo api(memo);

    EXPECT_EQ(api.getMemoType(), api::StellarLikeMemoType::MEMO_TEXT);
    EXPECT_EQ(api.getMemoText(), boost::get<std::string>(memo.content));
    EXPECT_EQ(api.memoValuetoString(), api.getMemoText());

    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoHash);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoId);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoReturn);

}

TEST_F(StellarFixture, GetMemoId) {
    stellar::xdr::Memo memo;
    memo.type = stellar::xdr::MemoType::MEMO_ID;
    memo.content = 12345678901234567890UL;

    StellarLikeMemo api(memo);

    EXPECT_EQ(api.getMemoType(), api::StellarLikeMemoType::MEMO_ID);
    EXPECT_EQ(api.getMemoId()->toString(10), fmt::format("{}", boost::get<uint64_t>(memo.content)));
    EXPECT_EQ(api.memoValuetoString(), api.getMemoId()->toString(10));

    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoHash);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoText);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoReturn);
}

TEST_F(StellarFixture, GetMemoHash) {
    auto bytesHex = std::string("1cc91667fc80e79caae59d3e5b29551ee528e9a4548307d1428901db71e459f6");
    auto bytesVector = hex::toByteArray(bytesHex);
    stellar::xdr::Hash bytesArray;
    std::copy(bytesVector.begin(), bytesVector.end(), bytesArray.begin());
    stellar::xdr::Memo memo;
    memo.type = stellar::xdr::MemoType::MEMO_HASH;
    memo.content = bytesArray;

    StellarLikeMemo api(memo);

    EXPECT_EQ(api.getMemoType(), api::StellarLikeMemoType::MEMO_HASH);
    EXPECT_EQ(hex::toString(api.getMemoHash()), bytesHex);
    EXPECT_EQ(api.memoValuetoString(), bytesHex);

    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoText);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoId);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoReturn);
}

TEST_F(StellarFixture, GetMemoReturn) {
    auto bytesHex = std::string("1cc91667fc80e79caae59d3e5b29551ee528e9a4548307d1428901db71e459f6");
    auto bytesVector = hex::toByteArray(bytesHex);
    stellar::xdr::Hash bytesArray;
    std::copy(bytesVector.begin(), bytesVector.end(), bytesArray.begin());
    stellar::xdr::Memo memo;
    memo.type = stellar::xdr::MemoType::MEMO_RETURN;
    memo.content = bytesArray;

    StellarLikeMemo api(memo);

    EXPECT_EQ(api.getMemoType(), api::StellarLikeMemoType::MEMO_RETURN);
    EXPECT_EQ(hex::toString(api.getMemoReturn()), bytesHex);
    EXPECT_EQ(api.memoValuetoString(), bytesHex);

    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoHash);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoText);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoId);
}

TEST_F(StellarFixture, GetMemoNone) {
    stellar::xdr::Memo memo;
    memo.type = stellar::xdr::MemoType::MEMO_NONE;

    StellarLikeMemo api(memo);

    EXPECT_EQ(api.getMemoType(), api::StellarLikeMemoType::MEMO_NONE);

    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoHash);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoText);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoId);
    expectThrowInvalidMemoType(&api, &StellarLikeMemo::getMemoReturn);
}