/*
 *
 * AbstractAccount
 * ledger-core
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

//#include <core/operation/OperationQuery.h>
#include <core/debug/LoggerApi.hpp>
#include <core/utils/Exception.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/events/Event.hpp>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/BlockDatabaseHelper.hpp>
#include <core/wallet/AccountDatabaseHelper.hpp>

namespace ledger {
    namespace core {
        AbstractAccount::AbstractAccount(
            const std::shared_ptr<Services>& services,
            const std::shared_ptr<AbstractWallet>& wallet,
            int32_t index
        ): DedicatedContext(services->getDispatcher()->getMainExecutionContext()),
           _services(services),
           _index(index),
           _uid(AccountDatabaseHelper::createAccountUid(wallet->getWalletUid(), index)),
           _logger(services->logger()),
           _internalPreferences(services
               ->getInternalPreferences()
               ->getSubPreferences(fmt::format("account_{}", index))
           ),
           _externalPreferences(services
               ->getExternalPreferences()
               ->getSubPreferences(fmt::format("account_{}", index))
           ),
           _loggerApi(std::make_shared<LoggerApi>(_logger)),
           _mainExecutionContext(services->getDispatcher()->getMainExecutionContext()),
           _wallet(wallet) {
           _publisher = std::make_shared<EventPublisher>(getContext());
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

        const std::string& AbstractAccount::getAccountUid() const {
            return _uid;
        }

        std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() const {
            auto wallet = _wallet.lock();
            if (!wallet) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Wallet was already released");
            }
            return wallet;
        }

        std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() {
            auto wallet = _wallet.lock();
            if (!wallet) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Wallet was already released");
            }
            return wallet;
        }

        const std::shared_ptr<api::ExecutionContext> AbstractAccount::getMainExecutionContext() const {
            return _mainExecutionContext;
        }

        //std::shared_ptr<api::OperationQuery> AbstractAccount::queryOperations() {
        //    return std::make_shared<OperationQuery>(
        //            api::QueryFilter::accountEq(getAccountUid()),
        //            _services->getDatabase(),
        //            getContext(),
        //            getMainExecutionContext()
        //    );
        //}

        std::shared_ptr<Preferences> AbstractAccount::getInternalPreferences() const {
            return _internalPreferences;
        }

        std::shared_ptr<Preferences> AbstractAccount::getExternalPreferences() const {
            return _externalPreferences;
        }

        void AbstractAccount::getFreshPublicAddresses(const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Address>>>, std::experimental::optional<api::Error>)> & callback) {
            getFreshPublicAddresses().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::getBalance(const std::function<void(std::shared_ptr<api::Amount>, std::experimental::optional<api::Error>)> & callback) {
            getBalance().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::getBalanceHistory(
            const std::string & start,
            const std::string & end,
            api::TimePeriod period,
            const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Amount>>>, std::experimental::optional<api::Error>)> & callback
        ) {
            getBalanceHistory(start, end, period).callback(getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::EventBus> AbstractAccount::getEventBus() {
            return _publisher->getEventBus();
        }

        void AbstractAccount::emitNewOperationEvent(const Operation &operation) {
            auto payload = DynamicObject::newInstance();
            payload->putString(api::Account::EV_NEW_OP_UID, operation.uid);
            payload->putString(api::Account::EV_NEW_OP_WALLET_NAME, getWallet()->getName());
            payload->putLong(api::Account::EV_NEW_OP_ACCOUNT_INDEX, getIndex());
            auto event = Event::newInstance(api::EventCode::NEW_OPERATION, payload);
            pushEvent(event);
        }

        void AbstractAccount::emitNewBlockEvent(const api::Block &block) {
            auto payload = DynamicObject::newInstance();
            payload->putLong(api::Account::EV_NEW_BLOCK_HEIGHT, block.height);
            payload->putString(api::Account::EV_NEW_BLOCK_HASH, block.blockHash);
            payload->putString(api::Account::EV_NEW_BLOCK_CURRENCY_NAME, block.currencyName);
            auto event = Event::newInstance(api::EventCode::NEW_BLOCK, payload);
            pushEvent(event);
        }

        void AbstractAccount::emitEventsNow() {
            auto self = shared_from_this();
            run([self] () {
                std::list<std::shared_ptr<api::Event>> events;
                {
                    std::lock_guard<std::mutex> lock(self->_eventsLock);
                    std::swap(events, self->_events);
                }
                for (auto& event : events) {
                    self->_publisher->post(event);
                }
            });
        }

        void AbstractAccount::pushEvent(const std::shared_ptr<api::Event> &event) {
            auto self = shared_from_this();
            run([event, self] () {
                std::lock_guard<std::mutex> lock(self->_eventsLock);
                self->_events.push_back(std::move(event));
            });
        }

        Future<api::Block> AbstractAccount::getLastBlock() {
            return getWallet()->getLastBlock();
        }

        void AbstractAccount::getLastBlock(const std::function<void(std::experimental::optional<api::Block>, std::experimental::optional<api::Error>)> & callback) {
            getLastBlock().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::eraseDataSince(const std::chrono::system_clock::time_point & date, const std::function<void(std::experimental::optional<api::ErrorCode>, std::experimental::optional<api::Error>)> & callback) {
            eraseDataSince(date).callback(getMainExecutionContext(), callback);
        }
    }
}
