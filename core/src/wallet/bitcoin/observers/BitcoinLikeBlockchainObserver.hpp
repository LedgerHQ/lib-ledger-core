/*
 *
 * BitcoinLikeBlockchainObserver
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
#ifndef LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP
#define LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP

#include <async/DedicatedContext.hpp>
#include <api/Currency.hpp>
#include <api/ExecutionContext.hpp>
#include <api/DynamicObject.hpp>
#include <debug/logger.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeAccount;
        class BitcoinLikeBlockchainObserver : public DedicatedContext, public ConfigurationMatchable {
        public:
            BitcoinLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext>& context,
                                          const std::shared_ptr<api::DynamicObject>& configuration,
                                          const std::shared_ptr<spdlog::logger>& logger,
                                          const api::Currency& currency,
                                          const std::vector<std::string>& matchableKeys);

            virtual bool registerAccount(const std::shared_ptr<BitcoinLikeAccount>& account);
            virtual bool unregisterAccount(const std::shared_ptr<BitcoinLikeAccount>& account);
            virtual bool isRegistered(const std::shared_ptr<BitcoinLikeAccount>& account);
            virtual bool isObserving() const;
            std::shared_ptr<spdlog::logger> logger() const;
            const api::Currency& getCurrency() const;
            std::shared_ptr<api::DynamicObject> getConfiguration() const;

        protected:
            virtual void onStart() = 0;
            virtual void onStop() = 0;

            void putTransaction(const BitcoinLikeBlockchainExplorerTransaction& tx);
            void putBlock(const BitcoinLikeBlockchainExplorer::Block& block);

        private:
            bool _isRegistered(std::lock_guard<std::mutex>& lock, const std::shared_ptr<BitcoinLikeAccount>& account);

        private:
            mutable std::mutex _lock;
            std::list<std::shared_ptr<BitcoinLikeAccount>> _accounts;
            api::Currency _currency;
            std::shared_ptr<api::DynamicObject> _configuration;
            std::shared_ptr<spdlog::logger> _logger;
        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP
