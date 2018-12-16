/*
 *
 * BitcoinLikeBlockchainObserver.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "BitcoinLikeBlockchainObserver.hpp"
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeBlockchainObserver::BitcoinLikeBlockchainObserver(
                const std::shared_ptr<api::ExecutionContext> &context,
                const std::shared_ptr<api::DynamicObject>& configuration,
                const std::shared_ptr<spdlog::logger>& logger,
                const api::Currency &currency,
                const std::vector<std::string>& matchableKeys) : DedicatedContext(context), ConfigurationMatchable(matchableKeys) {
            _currency = currency;
            _configuration = configuration;
            setConfiguration(configuration);
            setLogger(logger);
        }

        void BitcoinLikeBlockchainObserver::putTransaction(const BitcoinLikeBlockchainExplorerTransaction &tx) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, tx] () {
                    soci::session sql(account->getWallet()->getDatabase()->getPool());
                    if (account->putTransaction(sql, tx) != BitcoinLikeAccount::FLAG_TRANSACTION_IGNORED)
                        account->emitEventsNow();
                });
            }
        }

        void BitcoinLikeBlockchainObserver::putBlock(const BitcoinLikeBlockchainExplorer::Block& block) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, block] () {
                    bool shouldEmitNow = false;
                    {
                        soci::session sql(account->getWallet()->getDatabase()->getPool());
                        shouldEmitNow = account->putBlock(sql, block);
                    }
                    if (shouldEmitNow)
                        account->emitEventsNow();
                });
            }
        }

    }
}