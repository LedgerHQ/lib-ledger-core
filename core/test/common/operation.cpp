/**
 *
 * main
 * ledger-core
 *
 * Created by Jérémy Coatelen on 28/01/2022.
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
#include "wallet/common/Operation.h"

using namespace ledger::core;


TEST(Operation, TezosUidIsValid) {
    Operation op{};
    op.tezosTransaction = TezosLikeBlockchainExplorerTransaction{};
    op.tezosTransaction->hash = "123";

    // start as transaction type (no additional)
    op.tezosTransaction->type = ledger::core::api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
    EXPECT_EQ(op.uid, "");
    op.refreshUid();
    EXPECT_EQ(op.uid, "5b46f1084ef236091f04e257ab1902ed564dac1cad6d8fb403c007e3fe838411");

    // change only type
    op.tezosTransaction->type = ledger::core::api::TezosOperationTag::OPERATION_TAG_REVEAL;
    op.refreshUid();
    EXPECT_EQ(op.uid, "79ac6452f6ab4ebd0cc6bc0041b89a8954849b032f70bcb21154a0b351565c98");

    // with additional
    op.refreshUid("456");
    EXPECT_EQ(op.uid, "c5a8f07c86e5442c58cd95c8e16de8f50c1b980cf591ab215edfb9389166843a");
}


TEST(Operation, TransactionIdFormat) {

    const std::string baseHash = {"test"};
    const std::string additional = {"add"};
    const auto opType = ledger::core::api::TezosOperationTag::OPERATION_TAG_TRANSACTION;;

    // simple case
    EXPECT_EQ(Operation::computeTransactionId(baseHash), baseHash);

    // with additional
    EXPECT_EQ(Operation::computeTransactionId(baseHash, additional), baseHash+"+"+additional);

    // with operation type
    EXPECT_EQ(Operation::computeTransactionId(baseHash, opType, additional), baseHash+"+"+api::to_string(opType)+"+"+additional);

    // now without additional
    EXPECT_EQ(Operation::computeTransactionId(baseHash, opType), baseHash+"+"+api::to_string(opType));
}