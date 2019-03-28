/*
 *
 * BalanceHistory
 *
 * Created by Dimitri Sabadie on 2019/03/27.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <memory>
#include <string>
#include <vector>

#include <api/OperationType.hpp>
#include <api/TimePeriod.hpp>
#include <async/Future.hpp>
#include <utils/DateUtils.hpp>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {
        namespace agnostic {
            // Get a balance history absade on a time window (inclusive) and currency operations.
            //
            // The Value type variable must be default-constructible and accept addition via the
            // plus (+) and minus (-) operators.
            //
            // The OperationIt type variable must implement the pre-increment operator (++it) and
            // be dereferencable with the * operator. It must have a value_type associated
            // type. Finally, it must be comparable with itself.
            template <typename Value, typename CastValue, typename ValueImpl, typename OperationIt>
            std::vector<std::shared_ptr<CastValue>> getBalanceHistoryFor(
                std::chrono::system_clock::time_point const& startDate,
                std::chrono::system_clock::time_point const& endDate,
                api::TimePeriod precision,
                OperationIt operationIt,
                OperationIt operationEnd,
                std::function<std::chrono::system_clock::time_point (typename OperationIt::value_type&)> opDate,
                std::function<api::OperationType (typename OperationIt::value_type&)> opType,
                std::function<Value (typename OperationIt::value_type&)> opValue,
                std::function<Option<Value> (typename OperationIt::value_type&)> opFees,
                Value zero
            ) {
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly greater than end date");
                }

                auto lowerDate = startDate;
                auto upperDate = DateUtils::incrementDate(startDate, precision);
                std::vector<std::shared_ptr<CastValue>> values;
                Value sum = zero;

                while (lowerDate <= endDate && operationIt != operationEnd) {
                    auto operation = *operationIt;
                    auto operationDate = opDate(operation);

                    while (operationDate > upperDate && lowerDate < endDate) {
                        lowerDate = DateUtils::incrementDate(lowerDate, precision);
                        upperDate = DateUtils::incrementDate(upperDate, precision);
                        values.emplace_back(std::make_shared<ValueImpl>(sum));
                    }

                    if (operationDate <= upperDate) {
                        switch (opType(operation)) {
                            case api::OperationType::RECEIVE: {
                                sum = sum + opValue(operation);
                                break;
                            }
                            case api::OperationType::SEND: {
                                sum = sum - (opValue(operation) + opFees(operation).getValueOr(zero));
                                break;
                            }
                        }
                    }

                    ++operationIt;
                }

                while (lowerDate < endDate) {
                    lowerDate = DateUtils::incrementDate(lowerDate, precision);
                    values.emplace_back(std::make_shared<ValueImpl>(sum));
                }

                return values;
            }
        }
    }
}
