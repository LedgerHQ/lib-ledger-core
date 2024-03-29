/*
 *
 * OperationQuery
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/07/2017.
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
#include "OperationQuery.h"

#include "Operation.h"

#include <api/OperationCountListCallback.hpp>
#include <api/OperationListCallback.hpp>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <wallet/algorand/database/AlgorandTransactionDatabaseHelper.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <wallet/ripple/database/RippleLikeTransactionDatabaseHelper.h>
#include <wallet/stellar/database/StellarLikeTransactionDatabaseHelper.hpp>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>

namespace ledger {
    namespace core {

        OperationQuery::OperationQuery(const std::shared_ptr<api::QueryFilter> &headFilter,
                                       const std::shared_ptr<DatabaseSessionPool> &pool,
                                       const std::shared_ptr<api::ExecutionContext> &context,
                                       const std::shared_ptr<api::ExecutionContext> &mainContext) : DedicatedContext(context) {
            _headFilter = headFilter;
            _builder.where(_headFilter);
            _fetchCompleteOperation = false;
            _pool                   = pool;
            _mainContext            = mainContext;
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::addOrder(api::OperationOrderKey key, bool descending) {
            switch (key) {
            case api::OperationOrderKey::AMOUNT:
                _builder.order("amount", std::move(descending), "o");
                break;
            case api::OperationOrderKey::DATE:
                _builder.order("date", std::move(descending), "o");
                break;
            case api::OperationOrderKey::SENDERS:
                _builder.order("senders", std::move(descending), "o");
                break;
            case api::OperationOrderKey::RECIPIENTS:
                _builder.order("recipients", std::move(descending), "o");
                break;
            case api::OperationOrderKey::TYPE:
                _builder.order("type", std::move(descending), "o");
                break;
            case api::OperationOrderKey::CURRENCY_NAME:
                _builder.order("currency_name", std::move(descending), "o");
                break;
            case api::OperationOrderKey::FEES:
                _builder.order("fees", std::move(descending), "o");
                break;
            case api::OperationOrderKey::BLOCK_HEIGHT:
                _builder.order("height", std::move(descending), "b");
                break;
            case api::OperationOrderKey::TIME:
                _builder.order("time", std::move(descending), "b"); // NOLINT
                break;
            }
            return shared_from_this();
        }

        std::shared_ptr<api::QueryFilter> OperationQuery::filter() {
            return _headFilter;
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::offset(int32_t from) {
            _builder.offset((int32_t)from);
            return shared_from_this();
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::limit(int32_t count) {
            _builder.limit((int32_t)count);
            return shared_from_this();
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::complete() {
            _fetchCompleteOperation = true;
            return shared_from_this();
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::partial() {
            _fetchCompleteOperation = false;
            return shared_from_this();
        }

        void OperationQuery::count(
            const std::shared_ptr<api::OperationCountListCallback> &callback) {
            count().callback(_mainContext, callback);
        }

        Future<std::vector<api::OperationCount>>
        OperationQuery::count() {
            auto self = shared_from_this();
            return async<std::vector<api::OperationCount>>([=]() {
                std::vector<api::OperationCount> out;
                self->performCount(out);
                return out;
            });
        }

        soci::rowset<soci::row> OperationQuery::performCount(soci::session &sql) {
            return _builder.select("o.type, count(*)")
                .from("operations")
                .to("o")
                .outerJoin("blocks AS b", "o.block_uid = b.uid")
                .groupBy("o.type")
                .execute(sql);
        }

        void OperationQuery::performCount(std::vector<api::OperationCount> &operations) {
            soci::session sql(_pool->getPool());
            const soci::rowset<soci::row> rows = performCount(sql);

            for (const auto &row : rows) {
                const auto type  = api::from_string<api::OperationType>(row.get<std::string>(0));
                const auto count = soci::get_number<int64_t>(row, 1);

                operations.emplace_back(api::OperationCount{type, count});
            }
        }

        void OperationQuery::execute(const std::shared_ptr<api::OperationListCallback> &callback) {
            execute().callback(_mainContext, callback);
        }

        Future<std::vector<std::shared_ptr<api::Operation>>>
        OperationQuery::execute() {
            auto self = shared_from_this();
            return async<std::vector<std::shared_ptr<api::Operation>>>([=]() {
                std::vector<std::shared_ptr<api::Operation>> out;
                self->performExecute(out);
                return out;
            });
        }

        soci::rowset<soci::row> OperationQuery::performExecute(soci::session &sql) {
            return _builder.select(
                               "o.account_uid, o.uid, o.wallet_uid, o.type, o.date, o.senders, o.recipients,"
                               "o.amount, o.fees, o.currency_name, o.trust, b.hash, b.height, b.time")
                .from("operations")
                .to("o")
                .outerJoin("blocks AS b", "o.block_uid = b.uid")
                .execute(sql);
        }

        void OperationQuery::performExecute(std::vector<std::shared_ptr<api::Operation>> &operations) {
            soci::session sql(_pool->getPool());
            soci::rowset<soci::row> rows = performExecute(sql);

            for (auto &row : rows) {
                auto accountUid = row.get<std::string>(0);
                auto account    = _accounts.find(accountUid);
                if (account == _accounts.end())
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Account {} is not registered.", accountUid);

                std::shared_ptr<OperationApi> operationApi;
                if (account->second->getWalletType() == api::WalletType::ALGORAND) {
                    operationApi = std::make_shared<algorand::Operation>(account->second);
                } else {
                    operationApi = std::make_shared<OperationApi>(account->second);
                }

                auto &operation        = operationApi->getBackend();

                // Inflate abstract operation

                operation.uid          = row.get<std::string>(1);
                operation.walletUid    = row.get<std::string>(2);
                operation.type         = api::from_string<api::OperationType>(row.get<std::string>(3));
                operation.date         = row.get<std::chrono::system_clock::time_point>(4);
                operation.senders      = strings::split(row.get<std::string>(5), ",");
                operation.recipients   = strings::split(row.get<std::string>(6), ",");
                operation.amount       = BigInt::fromHex(row.get<std::string>(7));
                operation.fees         = BigInt::fromHex(row.get<std::string>(8));
                operation.currencyName = row.get<std::string>(9);
                operation.trust        = nullptr;
                operation.walletType   = account->second->getWalletType();

                if (row.get_indicator(11) != soci::i_null) {
                    // The operation has a block, inflate the block
                    Block block;
                    block.hash         = row.get<std::string>(11);
                    block.height       = soci::get_number<uint64_t>(row, 12);
                    block.time         = row.get<std::chrono::system_clock::time_point>(13);
                    block.currencyName = operation.currencyName;
                    operation.block    = Option<Block>(std::move(block));
                }

                // End of inflate
                if (_fetchCompleteOperation) {
                    inflateCompleteTransaction(sql, accountUid, *operationApi);
                }
                operations.push_back(operationApi);
            }
        }

        std::shared_ptr<OperationQuery>
        OperationQuery::registerAccount(const std::shared_ptr<AbstractAccount> &account) {
            _accounts[account->getAccountUid()] = account;
            return shared_from_this();
        }

        void OperationQuery::inflateCompleteTransaction(soci::session &sql, const std::string &accountUid, OperationApi &operation) {
            switch (operation.getAccount()->getWalletType()) {
            case (api::WalletType::BITCOIN): return inflateBitcoinLikeTransaction(sql, accountUid, operation);
            case (api::WalletType::COSMOS): return inflateCosmosLikeTransaction(sql, accountUid, operation);
            case (api::WalletType::ETHEREUM): return inflateEthereumLikeTransaction(sql, operation);
            case (api::WalletType::RIPPLE): return inflateRippleLikeTransaction(sql, operation);
            case (api::WalletType::TEZOS): return inflateTezosLikeTransaction(sql, operation);
            case (api::WalletType::MONERO): return inflateMoneroLikeTransaction(sql, operation);
            case (api::WalletType::STELLAR): return inflateStellarLikeTransaction(sql, operation);
            case (api::WalletType::ALGORAND): return inflateAlgorandLikeTransaction(sql, dynamic_cast<algorand::Operation &>(operation));
            }
        }

        void OperationQuery::inflateBitcoinLikeTransaction(soci::session &sql, const std::string &accountUid, OperationApi &operation) {
            BitcoinLikeBlockchainExplorerTransaction tx;
            operation.getBackend().bitcoinTransaction = Option<BitcoinLikeBlockchainExplorerTransaction>(tx);
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM bitcoin_operations WHERE uid = :uid", soci::use(operation.getBackend().uid), soci::into(transactionHash);
            BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, transactionHash, accountUid, operation.getBackend().bitcoinTransaction.getValue());
        }

        void OperationQuery::inflateCosmosLikeTransaction(
            soci::session &sql,
            const std::string &accountUid,
            OperationApi &operation) {
            cosmos::Transaction tx;
            cosmos::Message msg;
            operation.getBackend().cosmosTransaction = Option<cosmos::OperationQueryResult>({tx, msg});
            std::string transactionHash;
            sql << "SELECT tx.hash "
                   "FROM cosmos_transactions AS tx "
                   "LEFT JOIN cosmos_messages AS msg ON msg.transaction_uid = tx.uid "
                   "LEFT JOIN cosmos_operations AS op ON op.message_uid = msg.uid "
                   "WHERE op.uid = :uid",
                soci::use(operation.getBackend().uid), soci::into(transactionHash);
            CosmosLikeTransactionDatabaseHelper::getTransactionByHash(
                sql, transactionHash, operation.getBackend().cosmosTransaction.getValue().tx);
            std::string msgUid;
            sql << "SELECT msg.uid "
                   "FROM cosmos_messages AS msg "
                   "LEFT JOIN cosmos_operations AS op ON op.message_uid = msg.uid "
                   "WHERE op.uid = :uid",
                soci::use(operation.getBackend().uid), soci::into(msgUid);

            CosmosLikeTransactionDatabaseHelper::getMessageByUid(
                sql, msgUid, operation.getBackend().cosmosTransaction.getValue().msg);
        }

        void OperationQuery::inflateRippleLikeTransaction(soci::session &sql, OperationApi &operation) {
            RippleLikeBlockchainExplorerTransaction tx;
            operation.getBackend().rippleTransaction = Option<RippleLikeBlockchainExplorerTransaction>(tx);
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM ripple_operations WHERE uid = :uid", soci::use(operation.getBackend().uid), soci::into(transactionHash);
            RippleLikeTransactionDatabaseHelper::getTransactionByHash(sql, transactionHash, operation.getBackend().rippleTransaction.getValue());
        }

        void OperationQuery::inflateTezosLikeTransaction(soci::session &sql, OperationApi &operation) {
            TezosLikeBlockchainExplorerTransaction tx;
            operation.getBackend().tezosTransaction = Option<TezosLikeBlockchainExplorerTransaction>(tx);
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM tezos_operations WHERE uid = :uid", soci::use(operation.getBackend().uid), soci::into(transactionHash);
            TezosLikeTransactionDatabaseHelper::getTransactionByHash(sql, transactionHash, operation.getBackend().uid, operation.getBackend().tezosTransaction.getValue());
        }

        void OperationQuery::inflateEthereumLikeTransaction(soci::session &sql, OperationApi &operation) {
            EthereumLikeBlockchainExplorerTransaction tx;
            operation.getBackend().ethereumTransaction = Option<EthereumLikeBlockchainExplorerTransaction>(tx);
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM ethereum_operations WHERE uid = :uid", soci::use(operation.getBackend().uid), soci::into(transactionHash);
            EthereumLikeTransactionDatabaseHelper::getTransactionByHash(sql, transactionHash, operation.getBackend().ethereumTransaction.getValue());
        }

        void OperationQuery::inflateMoneroLikeTransaction(soci::session &sql, OperationApi &operation) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Implement void OperationQuery::inflateMoneroLikeTransaction(soci::session &sql, OperationApi &operation)");
        }

        void OperationQuery::inflateStellarLikeTransaction(soci::session &sql, OperationApi &operation) {
            stellar::OperationWithParentTransaction out;
            StellarLikeTransactionDatabaseHelper::getOperation(sql, operation.getBackend().uid, out.operation);
            StellarLikeTransactionDatabaseHelper::getTransaction(sql, out.operation.transactionHash, out.transaction);
            operation.getBackend().stellarOperation = out;
        }

        void OperationQuery::inflateAlgorandLikeTransaction(soci::session &sql, algorand::Operation &operation) {
            std::string transactionHash;
            sql << "SELECT transaction_hash FROM algorand_operations WHERE uid = :uid",
                soci::use(operation.getBackend().uid),
                soci::into(transactionHash);

            algorand::model::Transaction tx;
            algorand::TransactionDatabaseHelper::getTransactionByHash(sql, transactionHash, tx);

            operation.setTransaction(tx);
        }

    } // namespace core
} // namespace ledger
