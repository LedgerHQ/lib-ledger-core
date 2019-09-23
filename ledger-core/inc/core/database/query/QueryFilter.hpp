/*
 *
 * QueryFilter
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/06/2017.
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

#include <soci.h>

#include <core/api/QueryFilter.hpp>

namespace ledger {
    namespace core {
        enum class QueryFilterOperator {
            OP_AND, OP_OR, OP_AND_NOT, OP_OR_NOT
        };

        class QueryFilter;
        struct QueryFilterLink {
            std::shared_ptr<QueryFilter> previous;
            std::shared_ptr<QueryFilter> next;
            QueryFilterOperator op;
        };

        class QueryFilter : public api::QueryFilter, public std::enable_shared_from_this<QueryFilter> {
        public:
            std::shared_ptr<api::QueryFilter> op_and(const std::shared_ptr<api::QueryFilter> &filter) override;
            std::shared_ptr<api::QueryFilter> op_or(const std::shared_ptr<api::QueryFilter> &filter) override;
            std::shared_ptr<api::QueryFilter> op_and_not(const std::shared_ptr<api::QueryFilter> &filter) override;
            std::shared_ptr<api::QueryFilter> op_or_not(const std::shared_ptr<api::QueryFilter> &filter) override;

            virtual std::string toString() const {
                std::stringstream ss;
                toString(ss);
                return ss.str();
            };
            virtual void toString(std::stringstream& ss) const = 0;
            virtual void bindValue(soci::details::prepare_temp_type& statement) const = 0;
            int32_t getSiblingsCount() const;
            std::shared_ptr<QueryFilter> getHead() const;
            std::shared_ptr<QueryFilter> getTail() const;
            bool isTail() const;
            bool isHead() const;

            std::shared_ptr<QueryFilter> getNext() const;
            std::shared_ptr<QueryFilter> getPrevious() const;
            QueryFilterOperator getOperatorForNextFilter() const;

        private:
            std::shared_ptr<api::QueryFilter> link(const std::shared_ptr<api::QueryFilter> &filter, QueryFilterOperator op);

        private:
            QueryFilterLink _siblings;
        };
    }
}
