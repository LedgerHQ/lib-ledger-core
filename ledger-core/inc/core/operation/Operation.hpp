
/*
 *
 * Operation
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/06/2017.
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

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <core/api/Block.hpp>
#include <core/api/Operation.hpp>
#include <core/api/OperationType.hpp>
#include <core/math/BigInt.hpp>
#include <core/utils/Option.hpp>
#include <core/wallet/TrustIndicator.hpp>

namespace ledger {
    namespace core {
        class AbstractAccount;

        struct Operation : api::Operation, std::enable_shared_from_this<Operation> {
            Operation() = default;
            Operation(std::shared_ptr<AbstractAccount> const& account);

            virtual ~Operation() = default;

            virtual void refreshUid() = 0;

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
            std::shared_ptr<AbstractAccount> const& getAccount() const;

            api::Currency getCurrency() override;

            bool isComplete() override;

            std::string uid;
            std::string accountUid;
            std::string walletUid;
            std::chrono::system_clock::time_point date;
            std::vector<std::string> senders;
            std::vector<std::string> recipients;
            BigInt amount;
            Option<BigInt> fees;
            Option<api::Block> block;
            std::string currencyName;
            api::OperationType type;
            std::shared_ptr<TrustIndicator> trust;
            std::shared_ptr<AbstractAccount> _account;
        };
    }
}
