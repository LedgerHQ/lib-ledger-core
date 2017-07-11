/*
 *
 * AbstractAccount
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/04/2017.
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
#ifndef LEDGER_CORE_ABSTRACTACCOUNT_HPP
#define LEDGER_CORE_ABSTRACTACCOUNT_HPP

#include <src/api/Account.hpp>
#include "AbstractWallet.hpp"
#include "OperationQuery.h"

namespace ledger {
    namespace core {
        class AbstractAccount : public DedicatedContext, public api::Account, public std::enable_shared_from_this<AbstractAccount> {
        public:
            AbstractAccount(const std::shared_ptr<AbstractWallet>& wallet, int32_t index);
            int32_t getIndex() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::Logger> getLogger() override;
            bool isInstanceOfBitcoinLikeAccount() override;
            bool isInstanceOfEthereumLikeAccount() override;
            bool isInstanceOfRippleLikeAccount() override;
            api::WalletType getWalletType() override;
            std::shared_ptr<api::Preferences> getOperationPreferences(const std::string &uid) override;
            virtual std::shared_ptr<Preferences> getOperationExternalPreferences(const std::string &uid);
            virtual std::shared_ptr<Preferences> getOperationInternalPreferences(const std::string &uid);
            virtual std::shared_ptr<spdlog::logger> logger() const;
            virtual const std::string& getAccountUid() const;
            virtual std::shared_ptr<const AbstractWallet> getWallet() const;
            const std::shared_ptr<api::ExecutionContext> getMainExecutionContext() const;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

        private:
            api::WalletType  _type;
            int32_t  _index;
            std::string _uid;
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::Logger> _loggerApi;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<api::ExecutionContext> _mainExecutionContext;
            std::weak_ptr<const AbstractWallet> _wallet;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTACCOUNT_HPP
