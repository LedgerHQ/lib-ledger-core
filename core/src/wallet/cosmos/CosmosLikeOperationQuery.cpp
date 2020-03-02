/*
 *
 * CosmosLikeOperationQuery
 *
 * Created by Gerry Agbobada 22/01/2020
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

// #include <wallet/cosmos/CosmosLikeOperationQuery.hpp>

// #include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>

// namespace ledger {
//     namespace core {

//         CosmosLikeOperationQuery::CosmosLikeOperationQuery(
//             const std::shared_ptr<api::QueryFilter>& headFilter,
//             const std::shared_ptr<DatabaseSessionPool>& pool,
//             const std::shared_ptr<api::ExecutionContext>& context,
//             const std::shared_ptr<api::ExecutionContext>& mainContext)
//             : OperationQuery(headFilter, pool, context, mainContext) {

//         }

//         void CosmosLikeOperationQuery::inflateCompleteTransaction(soci::session &sql,
//                                                                   const std::string &accountUid,
//                                                                   CosmosLikeOperation& operation) {

//             // TODO Optimize sql

//             {
//                 ledger::core::cosmos::Transaction tx;

//                 std::string txHash;
//                 sql << "SELECT tx.hash "
//                     "FROM cosmos_transactions AS tx "
//                     "LEFT JOIN cosmos_messages AS msg ON msg.transaction_uid = tx.uid "
//                     "LEFT JOIN cosmos_operations AS op ON op.message_uid = msg.uid "
//                     "WHERE op.uid = :uid",
//                     soci::use(operation.getUid()),
//                     soci::into(txHash);

//                 CosmosLikeTransactionDatabaseHelper::getTransactionByHash(sql, txHash, tx);

//                 operation.setTransactionData(tx);
//             }

//             {
//                 ledger::core::cosmos::Message msg;

//                 std::string msgUid;
//                 sql << "SELECT msg.uid "
//                     "FROM cosmos_messages AS msg "
//                     "LEFT JOIN cosmos_operations AS op ON op.message_uid = msg.uid "
//                     "WHERE op.uid = :uid",
//                     soci::use(operation.getUid()),
//                     soci::into(msgUid);

//                 CosmosLikeTransactionDatabaseHelper::getMessageByUid(sql, msgUid, msg);

//                 operation.setMessageData(msg);
//             }
//         }

//         std::shared_ptr<CosmosLikeOperation> CosmosLikeOperationQuery::createOperation(std::shared_ptr<AbstractAccount> &account) {
//             return std::make_shared<CosmosLikeOperation>();
//         }
//     }
// }
