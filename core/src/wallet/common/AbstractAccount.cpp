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
#include <wallet/common/database/AccountDatabaseHelper.h>
#include "AbstractAccount.hpp"
#include <wallet/common/OperationQuery.h>
#include <api/StringListCallback.hpp>
#include <api/AmountCallback.hpp>

namespace ledger {
    namespace core {

        AbstractAccount::AbstractAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index)
                : DedicatedContext(wallet->getMainExecutionContext()) {
            _uid = AccountDatabaseHelper::createAccountUid(wallet->getWalletUid(), index);
            _logger = wallet->logger();
            _index = index;
            _internalPreferences = wallet->getAccountInternalPreferences(index);
            _externalPreferences = wallet->getAccountExternalPreferences(index);
            _loggerApi = wallet->getLogger();
            _wallet = wallet;
            _mainExecutionContext = wallet->getMainExecutionContext();
            _logger = wallet->logger();
            _type = wallet->getWalletType();
        }

        int32_t AbstractAccount::getIndex() {
            return _index;
        }

        std::shared_ptr<api::Preferences> AbstractAccount::getPreferences() {
            return _externalPreferences;
        }

        std::shared_ptr<api::Logger> AbstractAccount::getLogger() {
            return _loggerApi;
        }

        bool AbstractAccount::isInstanceOfBitcoinLikeAccount() {
            return _type == api::WalletType::BITCOIN;
        }

        bool AbstractAccount::isInstanceOfEthereumLikeAccount() {
            return _type == api::WalletType::ETHEREUM;
        }

        bool AbstractAccount::isInstanceOfRippleLikeAccount() {
            return _type == api::WalletType::RIPPLE;
        }

        api::WalletType AbstractAccount::getWalletType() {
            return _type;
        }

        std::shared_ptr<api::Preferences> AbstractAccount::getOperationPreferences(const std::string &uid) {
            return getOperationExternalPreferences(uid);
        }

        std::shared_ptr<spdlog::logger> AbstractAccount::logger() const {
            return _logger;
        }

        std::shared_ptr<Preferences> AbstractAccount::getOperationExternalPreferences(const std::string &uid) {
            return _externalPreferences->getSubPreferences(fmt::format("operation_{}", uid));
        }

        std::shared_ptr<Preferences> AbstractAccount::getOperationInternalPreferences(const std::string &uid) {
            return _internalPreferences->getSubPreferences(fmt::format("operation_{}", uid));
        }

        const std::string &AbstractAccount::getAccountUid() const {
            return _uid;
        }

        std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() const {
            return _wallet.lock();
        }

        std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() {

            return _wallet.lock();
        }

        const std::shared_ptr<api::ExecutionContext> AbstractAccount::getMainExecutionContext() const {
            return _mainExecutionContext;
        }

        std::shared_ptr<api::OperationQuery> AbstractAccount::queryOperations() {
            return std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getContext(),
                    getMainExecutionContext()
            );
        }

        std::shared_ptr<Preferences> AbstractAccount::getInternalPreferences() const {
            return _internalPreferences;
        }

        std::shared_ptr<Preferences> AbstractAccount::getExternalPreferences() const {
            return _externalPreferences;
        }

        void AbstractAccount::getFreshPublicAddresses(const std::shared_ptr<api::StringListCallback> &callback) {
            getFreshPublicAddresses().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::getBalance(const std::shared_ptr<api::AmountCallback> &callback) {
            getBalance().callback(getMainExecutionContext(), callback);
        }

    }
}