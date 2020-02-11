/*
 *
 * TezosLikeOriginatedAccount
 *
 * Created by El Khalil Bellakrid on 17/05/2019.
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

#include <memory>

#include <tezos/TezosLikeCurrencies.hpp>
#include <tezos/delegation/TezosLikeOriginatedAccount.hpp>
#include <tezos/operations/TezosLikeOperationQuery.hpp>

#include <core/api/Amount.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/api/Operation.hpp>
#include <core/api/OperationOrderKey.hpp>
#include <core/database/query/ConditionQueryFilter.hpp>
#include <core/wallet/BalanceHistory.hpp>

namespace ledger {
    namespace core {
        TezosLikeOriginatedAccount::TezosLikeOriginatedAccount(const std::string &uid,
                                                               const std::string &address,
                                                               const std::shared_ptr<TezosLikeAccount> &originatorAccount,
                                                               bool isSpendable,
                                                               bool isDelegatable,
                                                               const Option<std::string> &publicKey) :
                _accountUid(uid),
                _address(address),
                _originatorAccount(originatorAccount),
                _isSpendable(isSpendable),
                _isDelegatable(isDelegatable),
                _publicKey(publicKey)
        {
        }

        std::string TezosLikeOriginatedAccount::getAccountUid() {
            return _accountUid;
        }

        std::string TezosLikeOriginatedAccount::getAddress() {
            return _address;
        }

        std::experimental::optional<std::string> TezosLikeOriginatedAccount::getPublicKey() {
            return _publicKey.toOptional();
        }

        void TezosLikeOriginatedAccount::getBalance(const std::shared_ptr<api::AmountCallback> & callback) {
            auto localAccount = _originatorAccount.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
            getBalance(localAccount->getMainExecutionContext()).callback(localAccount->getMainExecutionContext(), callback);
        }

        FuturePtr<api::Amount> TezosLikeOriginatedAccount::getBalance(const std::shared_ptr<api::ExecutionContext>& context) {
            return std::dynamic_pointer_cast<TezosLikeOperationQuery>(queryOperations()->complete())->execute()
                    .mapPtr<api::Amount>(context, [] (const std::vector<std::shared_ptr<api::Operation>> &ops) {
                auto result = BigInt::ZERO;
                for (auto &op : ops) {
                    auto ty = op->getOperationType();
                    auto value = BigInt(op->getAmount()->toString());
                    switch (ty) {
                        case api::OperationType::RECEIVE:
                            result = result + value;
                            break;
                        case api::OperationType::SEND: {
                            auto fees = BigInt(op->getFees()->toString());
                            result = result - (value + fees);
                            break;
                        }
                        default:
                            break;
                    }
                }
                return std::make_shared<Amount>(currencies::TEZOS, 0, result);
            });
        }


        void TezosLikeOriginatedAccount::getBalanceHistory(
            const std::chrono::system_clock::time_point & start,
            const std::chrono::system_clock::time_point & end,
            api::TimePeriod period,
            const std::shared_ptr<api::AmountListCallback> & callback
        ) {
            auto localAccount = _originatorAccount.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
            getBalanceHistory(localAccount->getMainExecutionContext(), start, end, period).callback(localAccount->getMainExecutionContext(), callback);
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        TezosLikeOriginatedAccount::getBalanceHistory(const std::shared_ptr<api::ExecutionContext>& context,
                                                      const std::chrono::system_clock::time_point & start,
                                                      const std::chrono::system_clock::time_point & end,
                                                      api::TimePeriod period) {
            bool descending = false;
            return std::dynamic_pointer_cast<TezosLikeOperationQuery>(queryOperations()->addOrder(api::OperationOrderKey::DATE, descending)->complete())->execute()
                    .map<std::vector<std::shared_ptr<api::Amount>>>(context, [=] (const std::vector<std::shared_ptr<api::Operation>> &ops) {
                struct OperationStrategy {

                    static inline std::chrono::system_clock::time_point date(std::shared_ptr<api::Operation>& op) {
                        return op->getDate();
                    }

                    static inline std::shared_ptr<api::Amount> value_constructor(const BigInt& v) {
                        return std::make_shared<Amount>(currencies::TEZOS, 0, v);
                    }

                    static inline void update_balance(std::shared_ptr<api::Operation>& op, BigInt& sum) {
                        auto value = BigInt(op->getAmount()->toString());
                        switch (op->getOperationType()) {
                            case api::OperationType::RECEIVE:
                                sum = sum + value;
                                break;

                            case api::OperationType::SEND: {
                                auto fees = BigInt(op->getFees()->toString());
                                sum = sum - (value + fees);
                                break;
                            }

                            default:
                                break;
                        }
                    }
                };

                return agnostic::getBalanceHistoryFor<OperationStrategy, BigInt, api::Amount>(
                        start,
                        end,
                        period,
                        ops.cbegin(),
                        ops.cend(),
                        BigInt::ZERO
                );

            });
        }

        bool TezosLikeOriginatedAccount::isSpendable() {
            return _isSpendable;
        }

        bool TezosLikeOriginatedAccount::isDelegatable() {
            return _isDelegatable;
        }

        std::shared_ptr<api::OperationQuery> TezosLikeOriginatedAccount::queryOperations() {
            auto localAccount = _originatorAccount.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
            auto filter = std::make_shared<ConditionQueryFilter<std::string>>("originated_account_uid", "=", _accountUid, "orig_op");
            auto query = std::make_shared<TezosLikeOperationQuery>(
                    filter,
                    localAccount->getWallet()->getDatabase(),
                    localAccount->getWallet()->getServices()->getThreadPoolExecutionContext(),
                    localAccount->getWallet()->getMainExecutionContext()
            );
            query->registerAccount(localAccount);
            return query;
        }

        void TezosLikeOriginatedAccount::setPublicKey(const std::string &publicKey) {
            _publicKey = publicKey;
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeOriginatedAccount::buildTransaction() {
            return _originatorAccount.lock()->buildTransaction(_address);
        }
    }
}
