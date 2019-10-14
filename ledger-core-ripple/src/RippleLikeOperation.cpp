/*
 *
 * RippleLikeOperation
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#include <core/operation/OperationDatabaseHelper.hpp>

#include <RippleLikeTransactionDatabaseHelper.hpp>
#include <RippleLikeTransaction.hpp>
#include <RippleLikeOperation.hpp>

namespace ledger {
    namespace core {
        RippleLikeOperation::RippleLikeOperation(
            std::shared_ptr<RippleLikeBlockchainExplorerTransaction> const & tx,
            api::Currency const & currency
        ): _transaction(std::make_shared<RippleLikeTransaction>(tx, currency)) {
        }

        std::shared_ptr<api::RippleLikeTransaction> RippleLikeOperation::getTransaction() {
            return _transaction;
        }

        void RippleLikeOperation::refreshUid() {
          uid = OperationDatabaseHelper::createUid(
              accountUid,
              _transaction->getHash(),
              getOperationType()
          );
        }

        RippleLikeOperationQuery::RippleLikeOperationQuery(
            const std::shared_ptr<api::QueryFilter>& headFilter,
            const std::shared_ptr<DatabaseSessionPool>& pool,
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<api::ExecutionContext>& mainContext
        ): OperationQuery(headFilter, pool, context, mainContext) {
        }


        std::shared_ptr<RippleLikeOperation> RippleLikeOperationQuery::createOperation(
            std::shared_ptr<AbstractAccount> &account
        ) {
            RippleLikeBlockchainExplorerTransaction tx;
            return std::make_shared<RippleLikeOperation>(
                std::make_shared<RippleLikeBlockchainExplorerTransaction>(tx),
                account->getWallet()->getCurrency()
            );
        }

        void RippleLikeOperationQuery::inflateCompleteTransaction(
            soci::session& sql,
            std::string const& accountUid,
            RippleLikeOperation& operation
        ) {
            std::string transactionHash;
            RippleLikeBlockchainExplorerTransaction tx;

            sql << "SELECT transaction_hash FROM ripple_operations WHERE uid = :uid", soci::use(operation.uid), soci::into(transactionHash);
            RippleLikeTransactionDatabaseHelper::getTransactionByHash(
                sql,
                transactionHash,
                tx
            );

            auto currency = _accounts[accountUid]->getWallet()->getCurrency();
            operation = RippleLikeOperation(std::make_shared<RippleLikeBlockchainExplorerTransaction>(tx), currency);
        }
    }
}
