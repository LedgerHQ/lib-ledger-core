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
#include <api/OperationListCallback.hpp>
#include "Operation.h"
#include "../../../../../../Qt/5.8/clang_64/lib/QtCore.framework/Headers/QFuture"

namespace ledger {
    namespace core {

        OperationQuery::OperationQuery(const std::shared_ptr<api::QueryFilter>& headFilter,
                                       const std::shared_ptr<DatabaseSessionPool>& pool,
                                       const std::shared_ptr<api::ExecutionContext>& context,
                                       const std::shared_ptr<api::ExecutionContext>& mainContext) : DedicatedContext(context) {
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
                case api::OperationOrderKey::DATE:
                    _builder.order("date", std::move(descending));
                case api::OperationOrderKey::SENDERS:
                    _builder.order("senders", std::move(descending));
                case api::OperationOrderKey::RECIPIENTS:
                    _builder.order("recipients", std::move(descending));
                case api::OperationOrderKey::TYPE:
                    _builder.order("type", std::move(descending));
                case api::OperationOrderKey::CURRENCY_NAME:
                    _builder.order("currency_name", std::move(descending));
                case api::OperationOrderKey::FEES:
                    _builder.order("fees", std::move(descending));
                case api::OperationOrderKey::BLOCK_HEIGHT:
                    _builder.order("block_height", std::move(descending));
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

        void OperationQuery::execute(const std::shared_ptr<api::OperationListCallback> &callback) {
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

        void OperationQuery::performExecute(std::vector<std::shared_ptr<api::Operation>> &operations) {
            soci::session sql(_pool->getPool());
            _builder.select("").from("operations");
        }

    }
}