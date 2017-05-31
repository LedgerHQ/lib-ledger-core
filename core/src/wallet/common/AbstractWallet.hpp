/*
 *
 * AbstractWallet
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/04/2017.
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
#ifndef LEDGER_CORE_ABSTRACTWALLET_HPP
#define LEDGER_CORE_ABSTRACTWALLET_HPP

#include <src/api/Wallet.hpp>
#include <src/api/Currency.hpp>
#include <src/api/Account.hpp>
#include <src/preferences/Preferences.hpp>
#include <src/async/DedicatedContext.hpp>
#include <src/events/EventPublisher.hpp>
#include <src/debug/logger.hpp>
#include <src/api/WalletType.hpp>
#include <src/database/DatabaseSessionPool.hpp>

namespace ledger {
    namespace core {

        class WalletPool;

        class AbstractWallet : public virtual api::Wallet, public DedicatedContext {
        public:
            AbstractWallet(const std::string& walletName,
                           const api::Currency& currency,
                           const std::shared_ptr<WalletPool>& pool);
            std::shared_ptr<api::EventBus> getEventBus() override;
            std::shared_ptr<api::Preferences> getPreferences() override;
            bool isInstanceOfBitcoinLikeWallet() override;
            bool isInstanceOfEthereumLikeWallet() override;
            bool isInstanceOfRippleLikeWallet() override;
            std::shared_ptr<api::Logger> getLogger() override;
            api::WalletType getWalletType() override;
            std::shared_ptr<api::Preferences> getAccountPreferences(int32_t index) override;

            api::Currency getCurrency() override;

        public:
            virtual std::shared_ptr<Preferences> getAccountExternalPreferences(int32_t index);
            virtual std::shared_ptr<Preferences> getAccountInternalPreferences(int32_t index);
            virtual std::shared_ptr<Preferences> getInternalPreferences() const;
            virtual std::shared_ptr<Preferences> getExternalPreferences() const;
            virtual std::shared_ptr<EventPublisher> getEventPublisher() const;
            virtual std::shared_ptr<spdlog::logger> logger() const;
            virtual std::shared_ptr<DatabaseSessionPool> getDatabase() const;

        private:
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::Logger> _loggerApi;
            std::shared_ptr<EventPublisher> _publisher;
            std::shared_ptr<Preferences> _externalPreferences;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<DatabaseSessionPool> _database;
            api::Currency _currency;
        };
    }
}


#endif //LEDGER_CORE_ABSTRACTWALLET_HPP
