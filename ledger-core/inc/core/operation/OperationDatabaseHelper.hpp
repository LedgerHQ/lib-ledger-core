/*
 *
 * OperationDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/05/2017.
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

#include <memory>
#include <type_traits>
#include <string>

#include <soci.h>

#include <core/api/OperationType.hpp>
#include <core/operation/Operation.hpp>
#include <core/collections/Strings.hpp>

namespace ledger {
    namespace core {

        namespace impl {
            template <typename T>
            using isOperation = std::enable_if_t<std::is_base_of<Operation, T>::value, bool>;
        }

        class OperationDatabaseHelper {
        public:
            static bool putOperation(soci::session& sql, const Operation& operation);
            
            static std::string createUid(const std::string& accountUid,
                                            const std::string& txId,
                                            const api::OperationType type);
            static void queryOperations(soci::session& sql, int32_t from, int32_t to,
                                        bool complete, bool excludeDropped, std::vector<Operation>& out);

            // uses hack from https://www.fluentcpp.com/2019/08/23/how-to-make-sfinae-pretty-and-robust/
            template <typename T, impl::isOperation<T> = true>
            static std::size_t queryOperations(soci::session &sql,
                                               const std::string &accountUid,
                                               std::vector<T>& operations,
                                               std::function<bool (const std::string& address)> filter)
            {
                using namespace soci;

                constexpr auto query = "SELECT op.amount, op.fees, op.type, op.date, op.senders, op.recipients"
                                    " FROM operations AS op "
                                    " WHERE op.account_uid = :uid ORDER BY op.date";
                
                rowset<row> rows = (sql.prepare << query, use(accountUid));

                auto filterList = [&] (const std::vector<std::string> &list) -> bool {
                    for (auto& elem : list) {
                        if (filter(elem)) {
                            return true;
                        }
                    }
                    return false;
                };

                std::size_t c = 0;
                for (auto& row : rows) {
                    auto type = api::from_string<api::OperationType>(row.get<std::string>(2));
                    auto senders = strings::split(row.get<std::string>(4), ",");
                    auto recipients = strings::split(row.get<std::string>(5), ",");
                    if ((type == api::OperationType::SEND && row.get_indicator(4) != i_null && filterList(senders)) ||
                        (type == api::OperationType::RECEIVE && row.get_indicator(5) != i_null && filterList(recipients))) {
                        operations.resize(operations.size() + 1);
                        auto& operation = operations[operations.size() - 1];
                        operation.amount = BigInt::fromHex(row.get<std::string>(0));
                        operation.fees = BigInt::fromHex(row.get<std::string>(1));
                        operation.type = type;
                        operation.date = DateUtils::fromJSON(row.get<std::string>(3));
                        c += 1;
                    }
                }
                return c;
            }
        };
    }
}