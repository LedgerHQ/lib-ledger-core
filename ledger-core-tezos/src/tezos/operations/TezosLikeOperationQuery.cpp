/*
 *
 * TezosLikeOperationQuery
 *
 * Created by Alexis Le Provost 2019/11/15.
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

#include <tezos/database/TezosLikeTransactionDatabaseHelper.hpp>
#include <tezos/operations/TezosLikeOperationQuery.hpp>

namespace ledger {
    namespace core {

        TezosLikeOperationQuery::TezosLikeOperationQuery(
            const std::shared_ptr<api::QueryFilter>& headFilter,
            const std::shared_ptr<DatabaseSessionPool>& pool,
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<api::ExecutionContext>& mainContext) 
            : OperationQuery(headFilter, pool, context, mainContext) {

            }

        soci::rowset<soci::row> TezosLikeOperationQuery::performExecute(soci::session &sql) {
            return _builder.select("o.account_uid, o.uid, o.wallet_uid, o.type, o.date, o.senders, o.recipients,"
                                            "o.amount, o.fees, o.currency_name, o.trust, b.hash, b.height, b.time, orig_op.uid"
                    )
                    .from("operations").to("o")
                    .outerJoin("blocks AS b", "o.block_uid = b.uid")
                    .outerJoin("tezos_originated_operations AS orig_op", "o.uid = orig_op.uid")
                    .execute(sql);
         }

        void TezosLikeOperationQuery::inflateCompleteTransaction(
                soci::session &sql, const std::string &accountUid, TezosLikeOperation& operation) {
            TezosLikeBlockchainExplorerTransaction tx;
                
            operation.setExplorerTransaction(tx);
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM tezos_operations WHERE uid = :uid"
                , soci::use(operation.getUid())
                , soci::into(transactionHash);
            TezosLikeTransactionDatabaseHelper::getTransactionByHash(
                sql, transactionHash, operation.getUid(), operation.getExplorerTransaction());
        }

        std::shared_ptr<TezosLikeOperation> TezosLikeOperationQuery::createOperation(std::shared_ptr<AbstractAccount> &account) {
            TezosLikeBlockchainExplorerTransaction tx;

            return std::make_shared<TezosLikeOperation>(account->getWallet(), tx);
        }
    }
}