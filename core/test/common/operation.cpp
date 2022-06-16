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

#include "wallet/common/Operation.h"

#include <gtest/gtest.h>

using namespace ledger::core;

TEST(Operation, TezosUidIsValid) {
    Operation op{};
    op.tezosTransaction       = TezosLikeBlockchainExplorerTransaction{};
    op.tezosTransaction->hash = "123";

    // start as transaction type (no additional)
    op.tezosTransaction->type = ledger::core::api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
    EXPECT_EQ(op.uid, "");
    op.refreshUid();
    EXPECT_EQ(op.uid, "95d272362380dd84f330edebc776f178e12e6a637cef852512115ac182889b95");

    // change only type
    op.tezosTransaction->type = ledger::core::api::TezosOperationTag::OPERATION_TAG_REVEAL;
    op.refreshUid();
    EXPECT_EQ(op.uid, "98e675229d3001a50e4dd7a527643af3658b985b05f74fe4ff03305dd41ccdc2");

    // with additional
    op.refreshUid("456");
    EXPECT_EQ(op.uid, "cdd40b9116abc9830fd12cd38a4801fadf519fc11746f66bedc0c5062eee9774");
}

TEST(Operation, TransactionIdFormat) {
    const std::string baseHash   = {"test"};
    const std::string additional = {"add"};
    const auto opType            = ledger::core::api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
    ;

    // simple case
    EXPECT_EQ(Operation::computeTransactionId(baseHash), baseHash);

    // with additional
    EXPECT_EQ(Operation::computeTransactionId(baseHash, additional), baseHash + "+" + additional);

    // with operation type
    EXPECT_EQ(Operation::computeTransactionId(baseHash, opType, additional), baseHash + "+" + api::to_string(opType) + "+" + additional);

    // now without additional
    EXPECT_EQ(Operation::computeTransactionId(baseHash, opType), baseHash + "+" + api::to_string(opType));
}