/*
 *
 * StellarLikeBlockchainExplorerAccountSynchronizer.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/07/2019.
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

#ifndef LEDGER_CORE_STELLARLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_HPP
#define LEDGER_CORE_STELLARLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_HPP

#include "StellarLikeAccountSynchronizer.h"
#include <wallet/stellar/explorers/StellarLikeBlockchainExplorer.hpp>
#include <wallet/pool/WalletPool.hpp>
#include <events/EventPublisher.hpp>

namespace ledger {
    namespace core {

        class StellarLikeBlockchainExplorerAccountSynchronizer
                : public StellarLikeAccountSynchronizer,
                  public DedicatedContext,
                  public std::enable_shared_from_this<StellarLikeBlockchainExplorerAccountSynchronizer> {
        public:
            /**
             * Save the current state of the synchronize to enable incremental update and keep current
             * pagination cursors, to avoid synchronizing everytime the full account.
             */
            struct SavedState {
                /**
                 * Current version of the algorithm, to discriminate all saved state and ensure proper migration.
                 */
                int algorithmVersion;
                /**
                 * Last paging token used by the synchronizer for synchronizing transactions.
                 */
                std::string transactionPagingToken;

                template<class Archive>
                void serialize(Archive & archive) {
                    archive(transactionPagingToken);
                };
            };

            StellarLikeBlockchainExplorerAccountSynchronizer(
                    const std::shared_ptr<WalletPool>& pool,
                    const std::shared_ptr<StellarLikeBlockchainExplorer>& explorer
                    );
            void reset(const std::shared_ptr<StellarLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            std::shared_ptr<ProgressNotifier<Unit> >
            synchronize(const std::shared_ptr<StellarLikeAccount> &account) override;

            bool isSynchronizing() const override;

        protected:
            void synchronizeAccount(const std::shared_ptr<StellarLikeAccount> &account);
            void synchronizeAccount(const std::shared_ptr<StellarLikeAccount>& account,
                                    const Option<SavedState>& state);
            void synchronizeTransactions(const std::shared_ptr<StellarLikeAccount>& account,
                                         const Option<SavedState>& state);
            inline void failSynchronization(const Exception& ex);
            inline void endSynchronization();


        private:
            std::shared_ptr<ProgressNotifier<Unit>> _notifier;
            std::shared_ptr<StellarLikeBlockchainExplorer> _explorer;
            std::shared_ptr<DatabaseSessionPool> _database;
        };
    }
}


#endif //LEDGER_CORE_STELLARLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_HPP
