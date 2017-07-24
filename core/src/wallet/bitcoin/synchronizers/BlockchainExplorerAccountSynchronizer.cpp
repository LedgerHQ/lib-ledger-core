/*
 *
 * BlockchainExplorerAccountSynchronizer
 * ledger-core
 *
 * Created by Pierre Pollastri on 26/05/2017.
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
#include "BlockchainExplorerAccountSynchronizer.h"
#include <mutex>
#include <async/wait.h>
#include <utils/DateUtils.hpp>

namespace ledger {
    namespace core {

        BlockchainExplorerAccountSynchronizer::BlockchainExplorerAccountSynchronizer(
                const std::shared_ptr<WalletPool> &pool,
                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer) :
            DedicatedContext(pool->getDispatcher()->getThreadPoolExecutionContext("synchronizers")) {
            _explorer = explorer;
        }

        const ProgressNotifier<Unit> &
        BlockchainExplorerAccountSynchronizer::synchronize(const std::shared_ptr<BitcoinLikeAccount> &account) {
            std::lock_guard<std::mutex> lock(_lock);
            if (!_notifier) {
                _currentAccount = account;
                _notifier = std::make_shared<ProgressNotifier<Unit>>();
                run([=] () {
                   performSynchronization(account);
                    std::lock_guard<std::mutex> l(_lock);
                    _notifier = nullptr;
                    _currentAccount = nullptr;
                });
            } else if (account != _currentAccount) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "This synchronizer is already in use");
            }
            return *_notifier;
        }

        bool BlockchainExplorerAccountSynchronizer::isSynchronizing() const {
            return _notifier != nullptr;
        }

        void BlockchainExplorerAccountSynchronizer::reset(const std::shared_ptr<BitcoinLikeAccount> &account,
                                                          const std::chrono::system_clock::time_point &toDate) {

        }

        void BlockchainExplorerAccountSynchronizer::performSynchronization(const std::shared_ptr<BitcoinLikeAccount> &account) {
            auto logger = account->logger();
            auto startDate = DateUtils::now();
            auto wallet = account->getWallet();

            logger->info("Starting synchronization for account#{} ({}) of wallet {} at {}", account->getIndex(),
                         account->getKeychain()->getRestoreKey(),
                        account->getWallet()->getName(), DateUtils::toJSON(startDate));

            auto duration = DateUtils::now().time_since_epoch() - startDate.time_since_epoch();
            logger->info("End synchronization for account#{} of wallet {} in {} ms", account->getIndex(),
                         account->getWallet()->getName(), duration.count());
        }


    }
}