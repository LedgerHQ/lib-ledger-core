/*
 *
 * OperationApi
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/07/2017.
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

#include <core/api/Operation.hpp>
#include <core/api/Currency.hpp>
#include <core/wallet/Operation.h>

namespace ledger {
    namespace core {
        class AbstractAccount;
        class OperationApi : public api::Operation, public std::enable_shared_from_this<OperationApi> {
        public:
            OperationApi(const std::shared_ptr<AbstractAccount>& account);
            std::string getUid() override;
            int32_t getAccountIndex() override;
            api::OperationType getOperationType() override;
            std::chrono::system_clock::time_point getDate() override;
            std::vector<std::string> getSenders() override;
            std::vector<std::string> getRecipients() override;
            std::shared_ptr<api::Amount> getAmount() override;
            std::shared_ptr<api::Amount> getFees() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::TrustIndicator> getTrust() override;
            optional<int64_t> getBlockHeight() override;
            bool isComplete() override;
            ledger::core::Operation& getBackend();
            const std::shared_ptr<AbstractAccount>& getAccount() const;

            api::Currency getCurrency() override;

        private:
            ledger::core::Operation _backend;
            std::shared_ptr<AbstractAccount> _account;
            std::shared_ptr<api::Amount> _fees;
            std::shared_ptr<api::Amount> _amount;
        };
    }
}
