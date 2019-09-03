/*
 *
 * Operation
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

#include <core/operation/Operation.hpp>
#include <core/wallet/Amount.h>
#include <core/wallet/AbstractAccount.hpp>

namespace ledger {
    namespace core {
        Operation::Operation(std::shared_ptr<AbstractAccount> const& account) {
            _account = account;
        }

        std::string Operation::getUid() {
            return uid;
        }

        int32_t Operation::getAccountIndex() {
            return _account->getIndex();
        }

        api::OperationType Operation::getOperationType() {
            return type;
        }

        std::chrono::system_clock::time_point Operation::getDate() {
            return date;
        }

        std::vector<std::string> Operation::getSenders() {
            return senders;
        }

        std::vector<std::string> Operation::getRecipients() {
            return recipients;
        }

        std::shared_ptr<api::TrustIndicator> Operation::getTrust() {
            return trust;
        }

        std::shared_ptr<api::Preferences> Operation::getPreferences() {
            return _account->getOperationExternalPreferences(accountUid);
        }

        optional<int64_t> Operation::getBlockHeight() {
            return block.template map<int64_t>([] (const api::Block& block) {
                return block.height;
            });
        }

        std::shared_ptr<api::Amount> Operation::getFees() {
            if (fees.nonEmpty())
                return std::make_shared<Amount>(_account->getWallet()->getCurrency(), 0, fees.getValue());
            else
                return nullptr;
        }

        std::shared_ptr<api::Amount> Operation::getAmount() {
            return std::make_shared<Amount>(_account->getWallet()->getCurrency(), 0, amount);
        }

        std::shared_ptr<AbstractAccount> const& Operation::getAccount() const {
            return _account;
        }

        api::Currency Operation::getCurrency() {
            return _account->getWallet()->getCurrency();
        }
    }
}
