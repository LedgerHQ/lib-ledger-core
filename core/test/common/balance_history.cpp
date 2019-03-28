/**
 *
 * main
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/09/2016.
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

#include <algorithm>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <vector>

#include <api/OperationType.hpp>
#include <api/TimePeriod.hpp>
#include <utils/DateUtils.hpp>
#include <utils/Option.hpp>
#include <wallet/common/BalanceHistory.hpp>

using namespace ledger::core;
using namespace std;
using namespace std::chrono;

struct DummyOperation {
    int32_t amount;
    api::OperationType ty;
    system_clock::time_point date;

    DummyOperation(int32_t amount_, api::OperationType ty_, system_clock::time_point date_)
        : amount(amount_), ty(ty_), date(date_) {
    }

    ~DummyOperation() = default;
};

TEST(BalanceHistory, ZeroesOutOfRange) {
    // a basic collections of “operations” with nothing
    std::vector<DummyOperation> operations;
    auto start = DateUtils::fromJSON("2019-01-01T00:00:00Z");
    auto end = DateUtils::fromJSON("2019-02-01T00:00:00Z");

    auto balances = agnostic::getBalanceHistoryFor<int32_t, int32_t, int32_t>(
        start,
        end,
        api::TimePeriod::DAY,
        operations.cbegin(),
        operations.cend(),
        [](DummyOperation& op) { return op.date; },
        [](DummyOperation& op) { return op.ty; },
        [](DummyOperation& op) { return op.amount; },
        [](DummyOperation&)  { return Option<int32_t>(); },
        0
    );

    auto size = balances.size();
    EXPECT_EQ(size, 31);

    auto all_zero = all_of(balances.cbegin(), balances.cend(), [](shared_ptr<int32_t> const& balance) {
        return *balance == 0;
    });
    EXPECT_TRUE(all_zero);
}

TEST(BalanceHistory, CorrectBalances) {
    // a basic collections of “operations” with nothing
    std::vector<DummyOperation> operations = {
        DummyOperation(10, api::OperationType::RECEIVE, DateUtils::fromJSON("2019-01-01T00:00:00Z")),
        DummyOperation(12, api::OperationType::RECEIVE, DateUtils::fromJSON("2019-01-01T00:01:00Z")),
        DummyOperation(5, api::OperationType::SEND, DateUtils::fromJSON("2019-01-02T04:00:00Z")),
        DummyOperation(3, api::OperationType::SEND, DateUtils::fromJSON("2019-01-02T04:30:00Z"))
    };

    auto start = DateUtils::fromJSON("2019-01-01T00:00:00Z");
    auto end = DateUtils::fromJSON("2019-02-01T00:00:00Z");

    auto balances = agnostic::getBalanceHistoryFor<int32_t, int32_t, int32_t>(
        start,
        end,
        api::TimePeriod::DAY,
        operations.cbegin(),
        operations.cend(),
        [](DummyOperation& op) { return op.date; },
        [](DummyOperation& op) { return op.ty; },
        [](DummyOperation& op) { return op.amount; },
        [](DummyOperation&)  { return Option<int32_t>(); },
        0
    );

    auto size = balances.size();
    EXPECT_EQ(size, 31);

    EXPECT_EQ(*balances[0], 22);
    EXPECT_EQ(*balances[1], 14);
}
