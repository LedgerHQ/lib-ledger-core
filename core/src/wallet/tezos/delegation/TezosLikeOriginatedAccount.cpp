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

        std::experimental::optional<std::string> TezosLikeOriginatedAccount::getPublicKey(){
            return _publicKey.toOptional();
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
                    localAccount->getWallet()->getContext(),
                    localAccount->getWallet()->getMainExecutionContext()
            );
            query->registerAccount(localAccount);
            return query;
        }

        void TezosLikeOriginatedAccount::setPublicKey(const std::string &publicKey) {
            _publicKey = publicKey;
        }
    }
}