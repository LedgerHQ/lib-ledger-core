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
            ConditionQueryFilter(const std::string& fieldName, const std::string& symbol, const T& value,
                                 const std::string& prefix) {
                _fieldName = std::move(fieldName);
                _symbol = std::move(symbol);
                _value = value;
                if (!prefix.empty()) {
                    _prefixedName = fmt::format("{}.{}", prefix, _fieldName);
                } else {
                    _prefixedName = _fieldName;
                }
            };

            void toString(std::stringstream &ss) const override {
                if (_symbol.find("NULL") != std::string::npos) {
                    ss << _prefixedName << " " << _symbol;
                } else {
                    ss << _prefixedName << " " << _symbol << " :" << _fieldName;
                }
                if (!isTail()) {
                    switch (getOperatorForNextFilter()) {
                        case QueryFilterOperator::OP_AND :
                            ss << " AND ";
                            break;
                        case QueryFilterOperator::OP_AND_NOT :
                            ss << " AND NOT ";
                            break;
                        case QueryFilterOperator::OP_OR :
                            ss << " OR ";
                            break;
                        case QueryFilterOperator::OP_OR_NOT :
                            ss << " OR NOT ";
                            break;
                    }
                    getNext()->toString(ss);
                }
            }

            void bindValue(soci::details::prepare_temp_type &statement) const override {
                if (_symbol.find("NULL") == std::string::npos) {
                    statement, soci::use(_value);
                }
                if (!isTail()) {
                    getNext()->bindValue(statement);
                }
            }

        private:
            std::string _fieldName;
            std::string _symbol;
            std::string _prefixedName;
            T _value;
        };

        class PlainTextConditionQueryFilter : public QueryFilter {
        public:
            PlainTextConditionQueryFilter(const std::string& condition) : _condition(std::move(condition)) {};

            void toString(std::stringstream &ss) const override;

            void bindValue(soci::details::prepare_temp_type &statement) const override;

        private:
            std::string _condition;
        };
    }
}


#endif //LEDGER_CORE_CONDITIONQUERYFILTER_H
