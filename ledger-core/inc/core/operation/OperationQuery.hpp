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

#pragma once

#include <core/api/OperationQuery.hpp>
#include <core/api/OperationOrderKey.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/database/query/QueryBuilder.h>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/operation/Operation.hpp>
#include <unordered_map>

namespace ledger {
    namespace core {
        class AbstractAccount;
        class OperationQuery : public api::OperationQuery, public std::enable_shared_from_this<OperationQuery>,
                               public DedicatedContext {
        public:
            OperationQuery(
                const std::shared_ptr<api::QueryFilter>& headFilter,
                const std::shared_ptr<DatabaseSessionPool>& pool,
                const std::shared_ptr<api::ExecutionContext>& context,
                const std::shared_ptr<api::ExecutionContext>& mainContext
            );

            std::shared_ptr<api::OperationQuery> addOrder(api::OperationOrderKey key, bool descending) override;
            std::shared_ptr<api::QueryFilter> filter() override;
            std::shared_ptr<api::OperationQuery> offset(int64_t from) override;
            std::shared_ptr<api::OperationQuery> limit(int64_t count) override;
            std::shared_ptr<api::OperationQuery> complete() override;
            std::shared_ptr<api::OperationQuery> partial() override;

            void execute(const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Operation>>>, std::experimental::optional<api::Error>)> & callback) override;
            Future<std::vector<std::shared_ptr<api::Operation>>> execute();

            std::shared_ptr<OperationQuery> registerAccount(const  std::shared_ptr<AbstractAccount>& account);

        private:
            void performExecute(std::vector<std::shared_ptr<api::Operation>>& operations);

        protected:
            virtual std::shared_ptr<Operation> createOperation(
                std::shared_ptr<AbstractAccount> &account
            ) = 0;

            virtual void inflateCompleteTransaction(
                soci::session& sql,
                const std::string &accountUid,
                Operation& operation
            ) = 0;

            virtual soci::rowset<soci::row> performExecute(soci::session &sql);

            QueryBuilder _builder;
            std::shared_ptr<api::QueryFilter> _headFilter;
            bool _fetchCompleteOperation;
            std::shared_ptr<api::ExecutionContext> _mainContext;
            std::shared_ptr<DatabaseSessionPool> _pool;
            std::unordered_map<std::string, std::shared_ptr<AbstractAccount>> _accounts;
        };
    }
}
