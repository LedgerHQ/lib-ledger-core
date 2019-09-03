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

#include <core/api/enum_from_string.hpp>
#include <core/database/soci-date.h>
#include <core/database/soci-option.h>
#include <core/database/soci-number.h>
#include <core/collections/strings.hpp>
#include <core/operation/OperationQuery.hpp>
#include <core/operation/Operation.hpp>
#include <core/wallet/AbstractAccount.hpp>

namespace ledger {
    namespace core {
        OperationQuery::OperationQuery(
            const std::shared_ptr<api::QueryFilter>& headFilter,
            const std::shared_ptr<DatabaseSessionPool>& pool,
            const std::shared_ptr<api::ExecutionContext>& context,
            const std::shared_ptr<api::ExecutionContext>& mainContext
        ): DedicatedContext(context) {
            _headFilter = headFilter;
            _builder.where(_headFilter);
            _fetchCompleteOperation = false;
            _pool = pool;
            _mainContext = mainContext;
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::addOrder(api::OperationOrderKey key, bool descending) {
            switch (key) {
                case api::OperationOrderKey::AMOUNT:
                    _builder.order("amount", std::move(descending));
                    break;
                case api::OperationOrderKey::DATE:
                    _builder.order("date", std::move(descending));
                    break;
                case api::OperationOrderKey::SENDERS:
                    _builder.order("senders", std::move(descending));
                    break;
                case api::OperationOrderKey::RECIPIENTS:
                    _builder.order("recipients", std::move(descending));
                    break;
                case api::OperationOrderKey::TYPE:
                    _builder.order("type", std::move(descending));
                    break;
                case api::OperationOrderKey::CURRENCY_NAME:
                    _builder.order("currency_name", std::move(descending));
                    break;
                case api::OperationOrderKey::FEES:
                    _builder.order("fees", std::move(descending));
                    break;
                case api::OperationOrderKey::BLOCK_HEIGHT:
                    _builder.order("block_height", std::move(descending));
                    break;
            }
            return shared_from_this();
        }

        std::shared_ptr<api::QueryFilter> OperationQuery::filter() {
            return _headFilter;
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::offset(int64_t from) {
            _builder.offset((int32_t) from);
            return shared_from_this();
        }

        std::shared_ptr<api::OperationQuery> OperationQuery::limit(int64_t count) {
            _builder.limit((int32_t) count);
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

        void OperationQuery::execute(const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Operation>>>, std::experimental::optional<api::Error>)> & callback) {
           execute().callback(_mainContext, callback);
        }

        Future<std::vector<std::shared_ptr<api::Operation>>>
        OperationQuery::execute() {
            auto self = shared_from_this();
            return async<std::vector<std::shared_ptr<api::Operation>>>([=] () {
                std::vector<std::shared_ptr<api::Operation>> out;
                self->performExecute(out);
                return out;
            });
        }

        soci::rowset<soci::row> OperationQuery::performExecute(soci::session &sql) {
            return _builder.select(
                            "o.account_uid, o.uid, o.wallet_uid, o.type, o.date, o.senders, o.recipients,"
                                    "o.amount, o.fees, o.currency_name, o.trust, b.hash, b.height, b.time"
                    )
                    .from("operations").to("o")
                    .outerJoin("blocks AS b", "o.block_uid = b.uid")
                    .execute(sql);
        }

        void OperationQuery::performExecute(std::vector<std::shared_ptr<api::Operation>> &operations) {
            soci::session sql(_pool->getPool());
            soci::rowset<soci::row> rows = performExecute(sql);

            for (auto& row : rows) {
                auto accountUid = row.get<std::string>(0);
                auto account = _accounts.find(accountUid);
                if (account == _accounts.end())
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Account {} is not registered.", accountUid);
                auto operation = createOperation(account->second);

                // Inflate abstract operation

                operation->uid = row.get<std::string>(1);
                operation->walletUid = row.get<std::string>(2);
                operation->type = api::from_string<api::OperationType >(row.get<std::string>(3));
                operation->date = row.get<std::chrono::system_clock::time_point>(4);
                operation->senders = strings::split(row.get<std::string>(5), ",");
                operation->recipients = strings::split(row.get<std::string>(6), ",");
                operation->amount = BigInt::fromHex(row.get<std::string>(7));
                operation->fees = BigInt::fromHex(row.get<std::string>(8));
                operation->currencyName = row.get<std::string>(9);
                operation->trust = nullptr;

                if (row.get_indicator(11) != soci::i_null) {
                    // The operation has a block, inflate the block
                    api::Block block;
                    block.blockHash = row.get<std::string>(11);
                    block.height = soci::get_number<uint64_t>(row, 12);
                    block.time = row.get<std::chrono::system_clock::time_point>(13);
                    block.currencyName = operation->currencyName;
                    operation->block = Option<api::Block>(std::move(block));
                }

                // End of inflate
                if (_fetchCompleteOperation) {
                    inflateCompleteTransaction(sql, accountUid, *operation);
                }

                operations.push_back(operation);
            }
        }

        std::shared_ptr<OperationQuery> OperationQuery::registerAccount(
            const std::shared_ptr<AbstractAccount> &account
        ) {
            _accounts[account->getAccountUid()] = account;
            return shared_from_this();
        }
    }
}
