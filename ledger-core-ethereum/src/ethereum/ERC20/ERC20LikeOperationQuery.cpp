/*
 *
 * ERC20LikeOperationQuery
 *
 * Created by Alexis Le Provost 08/10/2019.
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

#include <ethereum/ERC20/ERC20LikeOperationQuery.hpp>

namespace ledger {
    namespace core {

        ERC20LikeOperationQuery::ERC20LikeOperationQuery(
            const std::shared_ptr<api::QueryFilter>& headFilter,
            const std::shared_ptr<DatabaseSessionPool>& pool,   
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<api::ExecutionContext>& mainContext) 
            : EthereumLikeOperationQuery(headFilter, pool, context, mainContext) {

        }

        soci::rowset<soci::row> ERC20LikeOperationQuery::performExecute(soci::session &sql) {
            return _builder
                .select(
                    "o.account_uid, o.uid, o.wallet_uid, o.type, o.date, o.senders, o.recipients,"
                    "o.amount, o.fees, o.currency_name, o.trust, b.hash, b.height, b.time, e.uid")
                .from("operations")
                .to("o")
                .outerJoin("blocks AS b", "o.block_uid = b.uid")
                .outerJoin("erc20_operations AS e", "o.uid = e.ethereum_operation_uid")
                .execute(sql);
        }
    }
}