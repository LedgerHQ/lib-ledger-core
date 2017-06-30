/*
 *
 * ConditionQueryFilter
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/06/2017.
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
#ifndef LEDGER_CORE_CONDITIONQUERYFILTER_H
#define LEDGER_CORE_CONDITIONQUERYFILTER_H

#include "QueryFilter.h"
#include <fmt/format.h>
#include <api/Amount.hpp>

namespace ledger {
    namespace core {
        template <typename T>
        class ConditionQueryFilter : public QueryFilter {
        public:
            ConditionQueryFilter(const std::string fieldName, const std::string symbol, const T& value) {
                _fieldName = std::move(fieldName);
                _symbol = std::move(symbol);
                _value = value;
            };
            std::string toString() const override {
                std::string op;
                if (!isTail()) {
                    switch (getOperatorForNextFilter()) {
                        case QueryFilterOperator::OP_AND :
                            op = " AND ";
                            break;
                        case QueryFilterOperator::OP_AND_NOT :
                            op = " AND NOT ";
                            break;
                        case QueryFilterOperator::OP_OR :
                            op = " OR ";
                            break;
                        case QueryFilterOperator::OP_OR_NOT :
                            op = " OR NOT ";
                            break;
                    }
                }
                auto format = fmt::format("{} {} :{}{}", _fieldName, _symbol, _fieldName, op);
                return isTail() ? format : format + getNext()->toString();
            }

            void bindValue(soci::details::prepare_temp_type &statement) const override {

            }

        private:
            std::string _fieldName;
            std::string _symbol;
            T _value;
        };
    }
}


#endif //LEDGER_CORE_CONDITIONQUERYFILTER_H
