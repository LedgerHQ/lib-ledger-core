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

#pragma once

#include <chrono>

#include <core/api/BigInt.hpp>
#include <core/api/Currency.hpp>
#include <core/database/SociNumber.hpp>
#include <core/operation/OperationQuery.hpp>

#include <ethereum/api/BigIntCallback.hpp>
#include <ethereum/api/BinaryCallback.hpp>
#include <ethereum/api/ERC20LikeAccount.hpp>
#include <ethereum/api/ERC20Token.hpp>
#include <ethereum/ERC20/ERC20LikeOperation.hpp>
#include <ethereum/EthereumLikeWallet.hpp>
#include <ethereum/EthereumLikeAccount.hpp>
#include <ethereum/database/EthereumLikeTransactionDatabaseHelper.hpp>
#include <ethereum/operations/EthereumLikeOperationDatabaseHelper.hpp>

namespace ledger {
    namespace core {
        class ERC20LikeAccount : public api::ERC20LikeAccount {
        public:
            ERC20LikeAccount(const std::string &accountUid,
                             const api::ERC20Token &erc20Token,
                             const std::string &accountAddress,
                             const api::Currency &parentCurrency,
                             const std::shared_ptr<EthereumLikeAccount> &parentAccount);
            api::ERC20Token getToken() override ;
            std::string getAddress() override ;
            FuturePtr<api::BigInt> getBalance();
            void getBalance(const std::shared_ptr<api::BigIntCallback> & callback) override ;

            std::vector<std::shared_ptr<api::BigInt>> getBalanceHistoryFor(
                const std::chrono::system_clock::time_point& startDate,
                const std::chrono::system_clock::time_point& endDate,
                api::TimePeriod period
            ) override;

            // A helper function to take an operation into an account while computing balances.
            static BigInt accumulateBalanceWithOperation(
                const BigInt& balance,
                api::ERC20LikeOperation& op
            );

            std::vector<std::shared_ptr<api::ERC20LikeOperation>> getOperations() override ;

            Future<std::vector<uint8_t>> getTransferToAddressData(const std::shared_ptr<api::BigInt> &amount,
                                                                  const std::string &address);

            void getTransferToAddressData(
                const std::shared_ptr<api::BigInt> &amount,
                const std::string &address,
                const std::shared_ptr<api::BinaryCallback> & data
            ) override;

            std::shared_ptr<api::OperationQuery> queryOperations() override ;
            void putOperation(soci::session &sql, const std::shared_ptr<ERC20LikeOperation> &operation, bool newOperation = false);
        private:
            std::shared_ptr<api::ExecutionContext> getContext();

            api::ERC20Token _token;
            std::string _accountAddress;
            api::Currency _parentCurrency;
            std::string _accountUid;
            std::weak_ptr<EthereumLikeAccount> _account;
        };
    }
}
