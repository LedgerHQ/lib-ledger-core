/*
 *
 * EthereumLikeAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
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


#ifndef LEDGER_CORE_ETHEREUMLIKEACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_ETHEREUMLIKEACCOUNTSYNCHRONIZER_H

#include <wallet/common/synchronizers/AbstractAccountSynchronizer.h>
#include <wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <wallet/pool/WalletPool.hpp>
#include <async/DedicatedContext.hpp>
#include <events/ProgressNotifier.h>


namespace ledger {
    namespace core {
        
        class EthereumLikeAccount;
        using EthereumBlockchainAccountSynchrinizer = AbstractAccountSynchronizer<EthereumLikeAccount, EthereumLikeAddress, EthereumLikeKeychain, EthereumLikeBlockchainExplorer>;
        class EthereumLikeAccountSynchronizer : public EthereumBlockchainAccountSynchrinizer,
                                                public DedicatedContext,
                                                public std::enable_shared_from_this<EthereumLikeAccountSynchronizer> {
        public:

            EthereumLikeAccountSynchronizer(const std::shared_ptr<WalletPool>& pool,
                                            const std::shared_ptr<EthereumLikeBlockchainExplorer>& explorer);

            void reset(const std::shared_ptr<EthereumLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            bool isSynchronizing() const override;
            void updateCurrentBlock(std::shared_ptr<AbstractAccountSynchronizer::SynchronizationBuddy> &buddy,
                                    const std::shared_ptr<api::ExecutionContext> &context) override;

            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr<SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

        private:
            std::shared_ptr<EthereumBlockchainAccountSynchrinizer> getSharedFromThis() override ;
            std::shared_ptr<api::ExecutionContext> getSynchronizerContext() override ;
        };
    }
}


#endif //LEDGER_CORE_ETHEREUMLIKEACCOUNTSYNCHRONIZER_H
