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

#include <core/api_impl/OperationApi.h>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/Amount.h>

namespace ledger {
    namespace core {
        OperationApi::OperationApi(const std::shared_ptr<AbstractAccount> &account) {
            _account = account;
        }

        std::string OperationApi::getUid() {
            return _backend.uid;
        }

        int32_t OperationApi::getAccountIndex() {
            return _account->getIndex();
        }

        api::OperationType OperationApi::getOperationType() {
            return _backend.type;
        }

        std::chrono::system_clock::time_point OperationApi::getDate() {
            return _backend.date;
        }

        std::vector<std::string> OperationApi::getSenders() {
            return _backend.senders;
        }

        std::vector<std::string> OperationApi::getRecipients() {
            return _backend.recipients;
        }

        Operation &OperationApi::getBackend() {
            return _backend;
        }

        bool OperationApi::isComplete() {
            return true;
        }

        std::shared_ptr<api::TrustIndicator> OperationApi::getTrust() {
            return _backend.trust;
        }

        std::shared_ptr<api::Preferences> OperationApi::getPreferences() {
            return _account->getOperationExternalPreferences(_backend.accountUid);
        }

        optional<int64_t> OperationApi::getBlockHeight() {
            return _backend.block.map<int64_t>([] (const Block& block) {
                return (int64_t) block.height;
            });
        }

        std::shared_ptr<api::Amount> OperationApi::getFees() {
            if (getBackend().fees.nonEmpty())
                return std::make_shared<Amount>(_account->getWallet()->getCurrency(), 0, getBackend().fees.getValue());
            else
                return nullptr;
        }

        std::shared_ptr<api::Amount> OperationApi::getAmount() {
            return std::make_shared<Amount>(_account->getWallet()->getCurrency(), 0, getBackend().amount);
        }

        const std::shared_ptr<AbstractAccount> &OperationApi::getAccount() const {
            return _account;
        }

        api::Currency OperationApi::getCurrency() {
            return _account->getWallet()->getCurrency();
        }

    }
}
