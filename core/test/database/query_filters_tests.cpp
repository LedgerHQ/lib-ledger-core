/*
 *
 * query_filters_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/06/2017.
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
#include <database/query/QueryFilter.h>
#include <api/TrustLevel.hpp>

using namespace ledger::core;

TEST(QueryFilters, SimpleFilter) {
    auto filter = api::QueryFilter::accountEq("my_account");
    EXPECT_EQ(std::dynamic_pointer_cast<QueryFilter>(filter)->toString(), "o.account_uid = :account_uid");
}

TEST(QueryFilters, DISABLED_DoubleConditionFilter) {
    auto filter = api::QueryFilter::accountEq("my_account")->op_and(api::QueryFilter::blockHeightGt(12000));
    EXPECT_EQ(std::dynamic_pointer_cast<QueryFilter>(filter)->getHead()->toString(), "o.account_uid = :account_uid AND b.height > :height");
}

TEST(QueryFilters, DISABLED_DoubleConditionWithCompoundFilter) {
    auto filter = api::QueryFilter::accountEq("my_account")
            ->op_and(api::QueryFilter::blockHeightGt(12000))
            ->op_or_not(api::QueryFilter::trustEq(api::TrustLevel::TRUSTED)->op_and(api::QueryFilter::containsSender("toto")));
    EXPECT_EQ(std::dynamic_pointer_cast<QueryFilter>(filter)->getHead()->toString(),
              "o.account_uid = :account_uid AND b.height > :height OR NOT (o.trust LIKE :trust AND o.senders LIKE :senders)");
}