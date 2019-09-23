/*
 *
 * AbstractBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 29/11/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <soci.h>

#include <core/debug/Logger.hpp>
#include <core/api/Currency.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/async/DedicatedContext.hpp>

namespace ledger {
    namespace core {
        template<typename Account, typename BlockchainExplorerTransaction, typename BlockchainExplorerBlock>
        class AbstractBlockchainObserver {
        public:
            virtual bool registerAccount(const std::shared_ptr<Account>& account) {
                std::lock_guard<std::mutex> lock(_lock);
                if (!_isRegistered(lock, account)) {
                    bool needsStart = _accounts.empty();
                    _accounts.push_front(account);
                    if (needsStart)
                        onStart();
                    return true;
                }
                return false;
            };
            virtual bool unregisterAccount(const std::shared_ptr<Account>& account) {
                std::lock_guard<std::mutex> lock(_lock);
                if (_isRegistered(lock, account)) {
                    bool needsStop = _accounts.size() == 1;
                    _accounts.remove(account);
                    if (needsStop)
                        onStop();
                    return true;
                }
                return false;
            };

            virtual bool isRegistered(const std::shared_ptr<Account>& account) {
                std::lock_guard<std::mutex> lock(_lock);
                return _isRegistered(lock, account);
            };

            std::shared_ptr<spdlog::logger> logger() const {
                return _logger;
            };

            bool isObserving() const {
                std::lock_guard<std::mutex> lock(_lock);
                return _accounts.size() > 0;
            };

        protected:
            virtual void onStart() = 0;
            virtual void onStop() = 0;

            virtual void putTransaction(const BlockchainExplorerTransaction& tx) = 0;
            virtual void putBlock(const BlockchainExplorerBlock& block) = 0;


            void setLogger(const std::shared_ptr<spdlog::logger>& logger) {
                _logger = logger;
            }

            std::list<std::shared_ptr<Account>> _accounts;
            mutable std::mutex _lock;
            std::shared_ptr<spdlog::logger> _logger;

        private:
            bool _isRegistered(std::lock_guard<std::mutex>& lock, const std::shared_ptr<Account>& account) {
                for (auto& acc : _accounts) {
                    if (acc.get() == account.get())
                        return true;
                }
                return false;
            };
        };
    }
}
