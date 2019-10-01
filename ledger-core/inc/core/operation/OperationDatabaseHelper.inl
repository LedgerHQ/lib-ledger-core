/*
 *
 * OperationDatabaseHelper
 * ledger-core
 *
 * Created by Alexis Le Provost on 01/10/2019.
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

#include <type_traits>

#include <core/api/Amount.hpp>
#include <core/api/BigInt.hpp>
#include <core/bytes/Serialization.hpp>
#include <core/crypto/SHA256.hpp>
#include <core/database/SociNumber.hpp>
#include <core/database/SociDate.hpp>
#include <core/database/SociOption.hpp>
#include <core/collections/Strings.hpp>
#include <core/operation/Operation.hpp>
#include <core/wallet/BlockDatabaseHelper.hpp>
#include <core/wallet/TrustIndicator.hpp>

namespace ledger {
    namespace core {

        namespace impl {

            // Declares a type trait to check if templated type has a static method
            // called `updateCurrencyOperation`.
            template <typename T, typename = void>
            struct has_update;

            // Uses SFINAE to determine if templated type has the 
            // `updateCurrencyOperation` method.
            // If the substitution succeed, `has_update` is evaluated as a
            // `true_type`
            template <typename T>
            struct has_update<T, decltype(T::updateCurrencyOperation)> : std::true_type
            {};

            // Else the `has_update` is evaluated as a `false_type`
            template <typename T>
            struct has_update<T, void> : std::false_type
            {};

            // Syntaxic sugar here to avoid to write the `has_update<T>::value`
            template <typename T>
            constexpr auto has_update_v = has_update<T>::value;
        }

        template <typename Derived>
        bool OperationDatabaseHelper<Derived>::putOperation(soci::session& sql, const Operation& operation)
        {
            static_assert(
                impl::has_update_v<Derived>,
                "OperationDatabaseHelper<T> requires 'T' to define 'updateCurrencyOperation' method");

            using namespace soci;

            auto count = 0;
            std::string serializedTrust;
            // TODO: I'm honestly not sure at all about this line below - since
            // api::TrustIndicator has none `serialize` method, we obviously need the
            // underlying implementation. Change `api::TrustIndicator` to
            // `TrustIndicator` in the `Operation` class is for sure a better solution 
            serialization::saveBase64<TrustIndicator>(*dynamic_cast<TrustIndicator*>(operation.trust.get()), serializedTrust);
            if (operation.block.nonEmpty()) {
                BlockDatabaseHelper::putBlock(sql, operation.block.getValue());
            }
            auto blockUid = operation.block.map<std::string>([] (const api::Block& block) {
                return block.uid;
            });
            sql << "SELECT COUNT(*) FROM operations WHERE uid = :uid", use(operation.uid), into(count);
            auto newOperation = count == 0;
            if (!newOperation) {
                sql << "UPDATE operations SET block_uid = :block_uid, trust = :trust WHERE uid = :uid"
                        , use(blockUid)
                        , use(serializedTrust)
                        , use(operation.uid);
                Derived::updateCurrencyOperation(sql, operation, newOperation);
                return false;
            } else {
                auto type = api::to_string(operation.type);
                std::stringstream senders;
                std::stringstream recipients;
                std::string separator(",");
                strings::join(operation.senders, senders, separator);
                strings::join(operation.recipients, recipients, separator);
                auto sndrs = senders.str();
                auto rcvrs = recipients.str();
                auto hexAmount = operation.amount.toHexString();
                auto hexFees = operation.fees.getValueOr(BigInt::ZERO).toHexString();
                sql << "INSERT INTO operations VALUES("
                            ":uid, :accout_uid, :wallet_uid, :type, :date, :senders, :recipients, :amount,"
                            ":fees, :block_uid, :currency_name, :trust"
                        ")"
                        , use(operation.uid), use(operation.accountUid), use(operation.walletUid), use(type), use(operation.date)
                        , use(sndrs), use(rcvrs), use(hexAmount)
                        , use(hexFees), use(blockUid)
                        , use(operation.currencyName), use(serializedTrust);

                Derived::updateCurrencyOperation(sql, operation, newOperation);
                return true;
            }

        }

        template <typename Derived>
        std::string OperationDatabaseHelper<Derived>::createUid(
            const std::string& accountUid, const std::string& txId, const api::OperationType type)
        {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}", accountUid, txId, api::to_string(type)));
        }
                
        template <typename Derived>
        void OperationDatabaseHelper<Derived>::queryOperations(
            soci::session&, int32_t, int32_t, bool, bool, std::vector<Operation>&)
        {
            throw std::runtime_error("Implementation is missing.");
        }

        template <typename Derived>
        std::size_t OperationDatabaseHelper<Derived>::queryOperations(
            soci::session &sql, 
            const std::string &accountUid, 
            std::vector<Operation>& operations,
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

    }
}