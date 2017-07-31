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
#ifndef LEDGER_CORE_BLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_BLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H

#include "BitcoinLikeAccountSynchronizer.hpp"
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <preferences/Preferences.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <async/DedicatedContext.hpp>

namespace ledger {
    namespace core {

        struct BlockchainExplorerAccountSynchronizationBatchSavedState {
            std::string blockHash;
            uint32_t blockHeight;

            BlockchainExplorerAccountSynchronizationBatchSavedState() {
                blockHeight = 0;
            }

            template<class Archive>
            void serialize(Archive & archive) {
                archive(blockHash, blockHeight);
            };
        };

        struct BlockchainExplorerAccountSynchronizationSavedState {
            uint32_t halfBatchSize;
            std::vector<BlockchainExplorerAccountSynchronizationBatchSavedState> batches;

            BlockchainExplorerAccountSynchronizationSavedState() {
                halfBatchSize = 0;
            }

            template<class Archive>
            void serialize(Archive & archive)
            {
                archive(halfBatchSize, batches); // serialize things by passing them to the archive
            }
        };

        class BlockchainExplorerAccountSynchronizer : public BitcoinLikeAccountSynchronizer,
                                                      public DedicatedContext,
                                                        public std::enable_shared_from_this<BlockchainExplorerAccountSynchronizer> {
        public:
            BlockchainExplorerAccountSynchronizer(
                    const std::shared_ptr<WalletPool>& pool,
                    const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer
            );
            void reset(const std::shared_ptr<BitcoinLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;
            const ProgressNotifier<Unit> &synchronize(const std::shared_ptr<BitcoinLikeAccount> &account) override;

            bool isSynchronizing() const override;

        private:

            struct SynchronizationBuddy {
                std::shared_ptr<Preferences> preferences;
                std::shared_ptr<spdlog::logger> logger;
                std::chrono::system_clock::time_point startDate;
                std::shared_ptr<AbstractWallet> wallet;
                std::shared_ptr<DynamicObject> configuration;
                uint32_t halfBatchSize;
                std::shared_ptr<BitcoinLikeKeychain> keychain;
                Option<BlockchainExplorerAccountSynchronizationSavedState> savedState;
                Option<void *> token;
                std::shared_ptr<BitcoinLikeAccount> account;
            };


            Future<Unit> performSynchronization(const std::shared_ptr<BitcoinLikeAccount> &account);
            Future<Unit> synchronizeBatches(uint32_t currentBatchIndex, std::shared_ptr<SynchronizationBuddy> buddy);
            Future<bool> synchronizeBatch(uint32_t currentBatchIndex,
                                          std::shared_ptr<SynchronizationBuddy> buddy,
                                          bool hadTransactions = false
            );
        private:
            std::shared_ptr<BitcoinLikeBlockchainExplorer> _explorer;
            std::shared_ptr<ProgressNotifier<Unit>> _notifier;
            std::shared_ptr<Preferences> _internalPreferences;
            std::shared_ptr<BitcoinLikeAccount> _currentAccount;
            std::mutex _lock;
        };
    }
}


#endif //LEDGER_CORE_BLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
