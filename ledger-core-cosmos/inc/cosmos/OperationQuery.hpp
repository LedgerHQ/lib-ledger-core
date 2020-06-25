/*
 *
 * CosmosLikeOperationQuery
 *
 * Created by Gerry Agbobada on 22/01/2020
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#pragma once

#include <core/operation/OperationQuery.hpp>
#include <cosmos/api_impl/Operation.hpp>
#include <cosmos/cosmos.hpp>
#include <cosmos/explorers/BlockchainExplorer.hpp>

// XXX : This file is very different from its design/modularization version
// Most of the code that was in design/modularization now lives in
// OperationQuery::inflateCosmosLikeTransaction.
// Also, specific CosmosLikeOperation constructors have been added to
// answer the Operation::asCosmosLikeOperation() calls.

namespace ledger {
namespace core {
namespace cosmos {
struct OperationQueryResult {
  Transaction tx;
  Message msg;
};

class CosmosLikeOperationQuery : public OperationQuery<CosmosLikeOperation> {
public:
  CosmosLikeOperationQuery(
      const std::shared_ptr<api::QueryFilter> &headFilter,
      const std::shared_ptr<DatabaseSessionPool> &pool,
      const std::shared_ptr<api::ExecutionContext> &context,
      const std::shared_ptr<api::ExecutionContext> &mainContext);

protected:
  soci::rowset<soci::row> performExecute(soci::session &sql) override;

  virtual void
  inflateCompleteTransaction(soci::session &sql, const std::string &accountUid,
                             CosmosLikeOperation &operation) override;

  virtual std::shared_ptr<CosmosLikeOperation>
  createOperation(std::shared_ptr<AbstractAccount> &account) override;
};
} // namespace cosmos
} // namespace core
} // namespace ledger
