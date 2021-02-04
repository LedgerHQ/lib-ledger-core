/*
 *
 * TezosLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/tezos/synchronizers/TezosLikeAccountSynchronizer.h>
#include <wallet/tezos/keychains/TezosLikeKeychain.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/pool/WalletPool.hpp>
#include <async/DedicatedContext.hpp>
#include <events/ProgressNotifier.h>

namespace ledger {
    namespace core {

        class TezosLikeAccount;

        using TezosBlockchainAccountSynchronizer = AbstractBlockchainExplorerAccountSynchronizer<TezosLikeAccount, TezosLikeAddress, TezosLikeKeychain, TezosLikeBlockchainExplorer>;

        class TezosLikeBlockchainExplorerAccountSynchronizer : public TezosBlockchainAccountSynchronizer,
                                                               public TezosLikeAccountSynchronizer,
                                                               public DedicatedContext,
                                                               public std::enable_shared_from_this<TezosLikeBlockchainExplorerAccountSynchronizer> {
        public:

            TezosLikeBlockchainExplorerAccountSynchronizer(const std::shared_ptr <WalletPool> &pool,
                                                           const std::shared_ptr <TezosLikeBlockchainExplorer> &explorer);

            void updateCurrentBlock(
                    std::shared_ptr <AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                    const std::shared_ptr <api::ExecutionContext> &context) override;

            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr <SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

            std::shared_ptr <ProgressNotifier<BlockchainExplorerAccountSynchronizationResult>>
            synchronize(const std::shared_ptr <TezosLikeAccount> &account) override;

            void reset(const std::shared_ptr <TezosLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            bool isSynchronizing() const override;

            void interpretTransaction(const Transaction& transaction,
                                      const std::shared_ptr<SynchronizationBuddy> &buddy,
                                      std::vector<Operation>& out) override;

        private:
            std::shared_ptr <TezosBlockchainAccountSynchronizer> getSharedFromThis() override;

            std::shared_ptr <api::ExecutionContext> getSynchronizerContext() override;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
