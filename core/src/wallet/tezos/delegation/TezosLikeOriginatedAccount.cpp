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


#include "TezosLikeOriginatedAccount.h"
#include <database/query/ConditionQueryFilter.h>
#include <api/ErrorCode.hpp>
#include <limits>
#include <wallet/currencies.hpp>
#include <wallet/common/BalanceHistory.hpp>
#include <api/Operation.hpp>
#include <api/OperationOrderKey.hpp>
#include <api/TezosLikeOperation.hpp>
#include <api/TezosLikeTransaction.hpp>
#include <wallet/pool/WalletPool.hpp>

namespace ledger {
    namespace core {
        // we need to compare the block height to the Babylon update height activation
        // because the way balances are computed difffers from pre- and post- Babylon
        // protocol activation
        //
        // <https://tezos.foundation/update-week-of-14-october-2019>
        const int64_t BABYLON_2_0_ACTIVATION_BLOCK_HEIGHT = 655361;

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
            getBalance(localAccount->getMainExecutionContext())
                    .callback(localAccount->getMainExecutionContext(), callback);
        }

        FuturePtr<api::Amount> TezosLikeOriginatedAccount::getBalance(const std::shared_ptr<api::ExecutionContext>& context) {
            return std::dynamic_pointer_cast<OperationQuery>(queryOperations()->complete())->execute().mapPtr<api::Amount>(context, [] (const std::vector<std::shared_ptr<api::Operation>> &ops) {
                auto result = BigInt::ZERO;
                for (auto &op : ops) {
                    auto ty = op->getOperationType();
                    auto value = BigInt(op->getAmount()->toString());
                    auto tzOp = op->asTezosLikeOperation();
                    auto tzTx = tzOp->getTransaction();

                    switch (ty) {
                        case api::OperationType::RECEIVE: {
                            // we discard received transactions that have failed
                            if (tzTx->getStatus() == 1) {
                                result = result + value;
                            }

                            break;
                        }

                        case api::OperationType::SEND: {
                            // if sending a transaction has failed:
                            //
                            // - Then we only spend the fees pre- Babylon 2.0.
                            // - We do nothing post- Babylon 2.0 as the fees will be spent by the
                            //   parent account.
                            auto isPreBabylon = op->getBlockHeight().value_or(std::numeric_limits<int64_t>::max()) < BABYLON_2_0_ACTIVATION_BLOCK_HEIGHT;

                            if (tzTx->getStatus() == 0) {
                                value = BigInt::ZERO;
                            }

                            if (isPreBabylon) {
                                auto fees = BigInt(op->getFees()->toString());
                                result = result - (value + fees);
                            } else {
                                // we do not spend the fees, as they are spent by the parent
                                // account
                                result = result - value;
                            }

                            break;
                        }
                        default:
                            break;
                    }
                }

                return std::make_shared<Amount>(currencies::TEZOS, 0, result);
            });
        }


        void TezosLikeOriginatedAccount::getBalanceHistory(const std::chrono::system_clock::time_point & start,
                                                           const std::chrono::system_clock::time_point & end,
                                                           api::TimePeriod period,
                                                           const std::shared_ptr<api::AmountListCallback> & callback) {
            auto localAccount = _originatorAccount.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
            getBalanceHistory(localAccount->getMainExecutionContext(), start, end, period)
                    .callback(localAccount->getMainExecutionContext(), callback);
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        TezosLikeOriginatedAccount::getBalanceHistory(const std::shared_ptr<api::ExecutionContext>& context,
                                                      const std::chrono::system_clock::time_point & start,
                                                      const std::chrono::system_clock::time_point & end,
                                                      api::TimePeriod period) {
            bool descending = false;
            return std::dynamic_pointer_cast<OperationQuery>(queryOperations()->addOrder(api::OperationOrderKey::DATE, descending)->complete())->execute().map<std::vector<std::shared_ptr<api::Amount>>>(context, [=] (const std::vector<std::shared_ptr<api::Operation>> &ops) {
                struct OperationStrategy {

                    static inline std::chrono::system_clock::time_point date(std::shared_ptr<api::Operation>& op) {
                        return op->getDate();
                    }

                    static inline std::shared_ptr<api::Amount> value_constructor(const BigInt& v) {
                        return std::make_shared<Amount>(currencies::TEZOS, 0, v);
                    }

                    static inline void update_balance(std::shared_ptr<api::Operation>& op, BigInt& sum) {
                        auto value = BigInt(op->getAmount()->toString());
                        auto tzOp = op->asTezosLikeOperation();
                        auto tzTx = tzOp->getTransaction();

                        switch (op->getOperationType()) {
                            case api::OperationType::RECEIVE: {
                                // we discard received transactions that have failed
                                if (tzTx->getStatus() == 1) {
                                  sum = sum + value;
                                }
                                break;
                            }

                            case api::OperationType::SEND: {
                                // if sending a transaction has failed:
                                //
                                // - Then we only spend the fees pre- Babylon 2.0.
                                // - We do nothing post- Babylon 2.0 as the fees will be spent by the
                                //   parent account.
                                auto isPreBabylon = op->getBlockHeight().value_or(std::numeric_limits<int64_t>::max()) < BABYLON_2_0_ACTIVATION_BLOCK_HEIGHT;

                                if (tzTx->getStatus() == 0) {
                                  value = BigInt::ZERO;
                                }

                                if (isPreBabylon) {
                                  auto fees = BigInt(op->getFees()->toString());
                                  sum = sum - (value + fees);
                                } else {
                                  // we do not spend the fees, as they are spent by the parent
                                  // account
                                  sum = sum - value;
                                }

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
            auto query = std::make_shared<TezosOriginatedOperationQuery>(
                    filter,
                    localAccount->getWallet()->getDatabase(),
                    localAccount->getWallet()->getPool()->getThreadPoolExecutionContext(),
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
