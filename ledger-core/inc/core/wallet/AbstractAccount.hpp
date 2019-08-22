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

#pragma once

#include <mutex>

#include <core/Services.hpp>
#include <core/api/Account.hpp>
#include <core/api/Address.hpp>
#include <core/api/Block.hpp>
#include <core/api/BlockCallback.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/TimePeriod.hpp>
#include <core/async/Future.hpp>
#include <core/events/EventPublisher.hpp>
#include <core/wallet/Amount.h>
#include <core/wallet/OperationQuery.h>

namespace ledger {
    namespace core {
        class AbstractAccount : public DedicatedContext, public api::Account, public std::enable_shared_from_this<AbstractAccount> {
        public:
            using AddressList = std::vector<std::shared_ptr<api::Address>>;

            AbstractAccount(const std::shared_ptr<Services>& services, int32_t index);
            int32_t getIndex() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            std::shared_ptr<api::Logger> getLogger() override;
            std::shared_ptr<api::Preferences> getOperationPreferences(const std::string &uid) override;
            virtual std::shared_ptr<Preferences> getOperationExternalPreferences(const std::string &uid);
            virtual std::shared_ptr<Preferences> getOperationInternalPreferences(const std::string &uid);
            virtual std::shared_ptr<Preferences> getInternalPreferences() const;
            virtual std::shared_ptr<Preferences> getExternalPreferences() const;
            virtual std::shared_ptr<spdlog::logger> logger() const;
            virtual const std::string& getAccountUid() const;
            const std::shared_ptr<api::ExecutionContext> getMainExecutionContext() const;

            void getLastBlock(const std::shared_ptr<api::BlockCallback> &callback) override;
            Future<api::Block> getLastBlock();

            void getBalance(const std::shared_ptr<api::AmountCallback> &callback) override;
            virtual FuturePtr<Amount> getBalance() = 0;

            void getFreshPublicAddresses(const std::shared_ptr<api::AddressListCallback> &callback) override;
            virtual Future<AddressList> getFreshPublicAddresses() = 0;
            void getBalanceHistory(const std::string & start,
                                   const std::string & end,
                                   api::TimePeriod precision,
                                   const std::shared_ptr<api::AmountListCallback> & callback) override;
            virtual Future<std::vector<std::shared_ptr<api::Amount>>> getBalanceHistory(const std::string & start,
                                                                                   const std::string & end,
                                                                                   api::TimePeriod precision) = 0;

            std::shared_ptr<api::OperationQuery> queryOperations() override;

            std::shared_ptr<api::EventBus> getEventBus() override;

            void emitEventsNow();

            void eraseDataSince(const std::chrono::system_clock::time_point & date, const std::shared_ptr<api::ErrorCodeCallback> & callback) override ;
            virtual Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date) = 0;

        protected:
            void emitNewOperationEvent(const Operation& operation);
            void emitNewBlockEvent(const Block& block);
            void pushEvent(const std::shared_ptr<api::Event>& event);

        private:
            int32_t _index;
            std::string _uid;
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::Logger> _loggerApi;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<api::ExecutionContext> _mainExecutionContext;
            std::shared_ptr<EventPublisher> _publisher;
            std::mutex _eventsLock;
            std::list<std::shared_ptr<api::Event>> _events;
        };
    }
}
