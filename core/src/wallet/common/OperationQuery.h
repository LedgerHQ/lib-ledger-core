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
#ifndef LEDGER_CORE_OPERATIONQUERY_H
#define LEDGER_CORE_OPERATIONQUERY_H

#include <api/OperationQuery.hpp>
#include <api/OperationOrderKey.hpp>
#include <database/query/QueryBuilder.h>
#include <database/DatabaseSessionPool.hpp>
#include <async/DedicatedContext.hpp>
#include "Operation.h"
#include "../common/api_impl/OperationApi.h"

namespace ledger {
    namespace core {
        class OperationQuery : public api::OperationQuery, public std::enable_shared_from_this<OperationQuery>,
                               public DedicatedContext {
        public:
            OperationQuery( const std::shared_ptr<api::QueryFilter>& headFilter,
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

            void execute(const std::shared_ptr<api::OperationListCallback> &callback) override;
            Future<std::vector<std::shared_ptr<api::Operation>>> execute();

        private:
            void performExecute(std::vector<std::shared_ptr<api::Operation>>& operations);
            void inflateBitcoinLikeOperation(std::vector<std::shared_ptr<api::Operation>>& operations);

        private:
            QueryBuilder _builder;
            std::shared_ptr<api::QueryFilter> _headFilter;
            bool _fetchCompleteOperation;
            std::shared_ptr<api::ExecutionContext> _mainContext;
            std::shared_ptr<DatabaseSessionPool> _pool;
        };
    }
}


#endif //LEDGER_CORE_OPERATIONQUERY_H
