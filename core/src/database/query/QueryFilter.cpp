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
#include "QueryFilter.h"
#include <utils/Exception.hpp>
#include "CompoundQueryFilter.h"

namespace ledger {
    namespace core {

        std::shared_ptr<api::QueryFilter> QueryFilter::op_and(const std::shared_ptr<api::QueryFilter> &filter) {
            return link(filter, QueryFilterOperator::OP_AND);
        }

        std::shared_ptr<api::QueryFilter> QueryFilter::op_or(const std::shared_ptr<api::QueryFilter> &filter) {
            return link(filter, QueryFilterOperator::OP_OR);
        }

        std::shared_ptr<api::QueryFilter> QueryFilter::op_and_not(const std::shared_ptr<api::QueryFilter> &filter) {
            return link(filter, QueryFilterOperator::OP_AND_NOT);
        }

        std::shared_ptr<api::QueryFilter> QueryFilter::op_or_not(const std::shared_ptr<api::QueryFilter> &filter) {
            return link(filter, QueryFilterOperator::OP_OR_NOT);
        }

        int32_t QueryFilter::getSiblingsCount() const {
            auto count = 0;
            auto ptr = getHead();
            while (ptr != nullptr) {
                ptr = ptr->_siblings.next;
                count += 1;
            }
            return count;
        }

        std::shared_ptr<api::QueryFilter> QueryFilter::link(const std::shared_ptr<api::QueryFilter> &filter, QueryFilterOperator op) {
            auto f = std::dynamic_pointer_cast<QueryFilter>(filter);
            std::shared_ptr<QueryFilter> newTail;

            if (!isTail()) {
                throw make_exception(api::ErrorCode::LINK_NON_TAIL_FILTER, "Cannot link already linked filter.");
            }

            if (f->getSiblingsCount() > 1) {
                // CREATE A COMPOUND FILTER
                newTail = std::make_shared<CompoundQueryFilter>(filter);
            } else {
                newTail = f;
            }
            _siblings.next = newTail;
            _siblings.op = op;
            newTail->_siblings.previous = shared_from_this();
            return newTail;
        }

        bool QueryFilter::isTail() const {
            return _siblings.next == nullptr;
        }

        std::shared_ptr<QueryFilter> QueryFilter::getHead() const {
            auto ptr = std::const_pointer_cast<QueryFilter>(shared_from_this());
            while (!ptr->isHead()) {
                ptr = ptr->_siblings.previous;
            }
            return ptr;
        }

        std::shared_ptr<QueryFilter> QueryFilter::getTail() const {
            auto ptr = std::const_pointer_cast<QueryFilter>(shared_from_this());
            while (!ptr->isTail()) {
                ptr = ptr->_siblings.next;
            }
            return ptr;
        }

        bool QueryFilter::isHead() const {
            return _siblings.previous == nullptr;
        }

        std::shared_ptr<QueryFilter> QueryFilter::getNext() const {
            return std::const_pointer_cast<QueryFilter>(shared_from_this())->_siblings.next;
        }

        std::shared_ptr<QueryFilter> QueryFilter::getPrevious() const {
            return std::const_pointer_cast<QueryFilter>(shared_from_this())->_siblings.previous;
        }

        QueryFilterOperator QueryFilter::getOperatorForNextFilter() const {
            return std::const_pointer_cast<QueryFilter>(shared_from_this())->_siblings.op;
        }
    }
}
