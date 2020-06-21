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

#include <wallet/bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <wallet/bitcoin/synchronizers/BitcoinLikeAccountSynchronizer.hpp>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/pool/WalletPool.hpp>
#include <preferences/Preferences.hpp>
#include <async/DedicatedContext.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeAccount;

        using BlockchainAccountSynchronizer = AbstractBlockchainExplorerAccountSynchronizer<BitcoinLikeAccount, BitcoinLikeAddress, BitcoinLikeKeychain, BitcoinLikeBlockchainExplorer>;
        class BlockchainExplorerAccountSynchronizer : public BitcoinLikeAccountSynchronizer,
                                                      public BlockchainAccountSynchronizer,
                                                      public DedicatedContext,
                                                      public std::enable_shared_from_this<BlockchainExplorerAccountSynchronizer> {
        public:
            BlockchainExplorerAccountSynchronizer(const std::shared_ptr<WalletPool>& pool,
                                                  const std::shared_ptr<BitcoinLikeBlockchainExplorer>& explorer);

            void updateCurrentBlock(std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                                    const std::shared_ptr<api::ExecutionContext> &context) override;
            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr<SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

            void reset(const std::shared_ptr<BitcoinLikeAccount>& account, const std::chrono::system_clock::time_point& toDate) override;
            std::shared_ptr<ProgressNotifier<Unit>> synchronize(const std::shared_ptr<BitcoinLikeAccount>& account) override;
            bool isSynchronizing() const override;

            int putTransaction(soci::session &sql, const Transaction &transaction,
                               const std::shared_ptr<SynchronizationBuddy> &buddy) override;

            std::shared_ptr<SynchronizationBuddy> makeSynchronizationBuddy() override;
            Future<Unit> synchronizeMempool(const std::shared_ptr<SynchronizationBuddy> &buddy) override;

            Future<Unit> recoverFromFailedSynchronization(const std::shared_ptr<SynchronizationBuddy> &buddy) override;

        private:
            std::shared_ptr<BlockchainAccountSynchronizer> getSharedFromThis() override ;
            std::shared_ptr<api::ExecutionContext> getSynchronizerContext() override ;

        };
    }
}

#endif //LEDGER_CORE_BLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
