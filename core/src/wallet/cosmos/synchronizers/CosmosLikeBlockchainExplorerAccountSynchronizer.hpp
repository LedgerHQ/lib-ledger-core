/*
 *
 * CosmosLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H


#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/pool/WalletPool.hpp>
#include <async/DedicatedContext.hpp>
#include <events/ProgressNotifier.h>

#include <wallet/cosmos/synchronizers/CosmosLikeAccountSynchronizer.hpp>
#include <wallet/cosmos/keychains/CosmosLikeKeychain.hpp>
#include <wallet/cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {

        class CosmosLikeAccount;

        using CosmosBlockchainAccountSynchronizer = AbstractBlockchainExplorerAccountSynchronizer<CosmosLikeAccount, CosmosLikeAddress, CosmosLikeKeychain, CosmosLikeBlockchainExplorer>;

        class CosmosLikeBlockchainExplorerAccountSynchronizer : public CosmosBlockchainAccountSynchronizer,
                                                                public CosmosLikeAccountSynchronizer,
                                                                public DedicatedContext,
                                                                public std::enable_shared_from_this<CosmosLikeBlockchainExplorerAccountSynchronizer> {
        public:

            CosmosLikeBlockchainExplorerAccountSynchronizer(const std::shared_ptr<WalletPool> &pool,
                                                            const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer);

            std::shared_ptr<ProgressNotifier<Unit>>
            synchronize(const std::shared_ptr<CosmosLikeAccount> &account) override;

            void reset(const std::shared_ptr<CosmosLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            bool isSynchronizing() const override;

            void updateCurrentBlock(
                    std::shared_ptr <AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                    const std::shared_ptr <api::ExecutionContext> &context) override;

            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr <SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

        private:
            std::shared_ptr<DatabaseSessionPool> _database;

            std::shared_ptr <CosmosBlockchainAccountSynchronizer> getSharedFromThis() override;

            std::shared_ptr <api::ExecutionContext> getSynchronizerContext() override;
        };
    }
}


#endif //LEDGER_CORE_COSMOSLIKEBLOCKCHAINEXPLORERACCOUNTSYNCHRONIZER_H
