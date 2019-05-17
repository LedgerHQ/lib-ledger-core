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


#ifndef LEDGER_CORE_TEZOSLIKEORIGINATEDACCOUNT_H
#define LEDGER_CORE_TEZOSLIKEORIGINATEDACCOUNT_H

#include <string>
#include <api/TezosLikeOriginatedAccount.hpp>
#include <api/OperationQuery.hpp>
#include <utils/Option.hpp>
#include <wallet/tezos/TezosLikeAccount.h>
#include <wallet/common/OperationQuery.h>
namespace ledger {
    namespace core {
        class TezosLikeOriginatedAccount : public api::TezosLikeOriginatedAccount {
        public:
            TezosLikeOriginatedAccount(const std::string &uid,
                                       const std::string &address,
                                       const std::shared_ptr<TezosLikeAccount> &originatorAccount,
                                       bool isSpendable,
                                       bool isDelegatable,
                                       const Option<std::string> &publicKey = Option<std::string>());

            std::string getAccountUid();

            std::string getAddress() override;

            std::experimental::optional<std::string> getPublicKey() override;

            bool isSpendable() override;

            bool isDelegatable() override;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            void setPublicKey(const std::string &publicKey);

        private:
            std::string _accountUid;
            std::string _address;
            Option<std::string> _publicKey;
            bool _isSpendable;
            bool _isDelegatable;
            std::weak_ptr<TezosLikeAccount> _originatorAccount;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEORIGINATEDACCOUNT_H
