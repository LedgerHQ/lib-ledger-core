/*
 *
 * ERC20LikeAccount
 *
 * Created by El Khalil Bellakrid on 26/08/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ERC20LIKEACCOUNT_H
#define LEDGER_CORE_ERC20LIKEACCOUNT_H

#include <api/ERC20LikeAccount.hpp>
#include <api/ERC20Token.hpp>
#include <api/ERC20LikeOperation.hpp>
#include <api/BigInt.hpp>
#include <api/Currency.hpp>

namespace ledger {
    namespace core {
        class ERC20LikeAccount : public api::ERC20LikeAccount {

        public:
            ERC20LikeAccount(const api::ERC20Token &erc20Token,
                             const std::string &accountAddress,
                            const api::Currency &parentCurrency);
            api::ERC20Token getToken() override ;
            std::string getAddress() override ;
            std::shared_ptr<api::BigInt> getBalance() override ;
            std::vector<std::shared_ptr<api::ERC20LikeOperation>> getOperations() override ;
            std::vector<uint8_t> getTransferToAddressData(const std::shared_ptr<api::Amount> & amount,
                                                          const std::string & address) override ;
            void putOperation(const std::shared_ptr<api::ERC20LikeOperation> &operation);

        private:
            api::ERC20Token _token;
            std::string _accountAddress;
            std::vector<std::shared_ptr<api::ERC20LikeOperation>> _operations;
            api::Currency _parentCurrency;
        };
    }
}


#endif //LEDGER_CORE_ERC20LIKEACCOUNT_H
