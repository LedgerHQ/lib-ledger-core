/*
 *
 * EthereumLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 29/07/2018.
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

#include <core/Services.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/events/ProgressNotifier.hpp>
#include <core/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.hpp>

#include <ethereum/synchronizers/EthereumLikeAccountSynchronizer.hpp>
#include <ethereum/keychains/EthereumLikeKeychain.hpp>
#include <ethereum/explorers/EthereumLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {

        class EthereumLikeAccount;
        using EthereumBlockchainAccountSynchronizer = AbstractBlockchainExplorerAccountSynchronizer<EthereumLikeAccount, EthereumLikeAddress, EthereumLikeKeychain, EthereumLikeBlockchainExplorer>;
        class EthereumLikeBlockchainExplorerAccountSynchronizer : public EthereumBlockchainAccountSynchronizer,
                                                                  public EthereumLikeAccountSynchronizer,
                                                                  public DedicatedContext,
                                                                  public std::enable_shared_from_this<EthereumLikeBlockchainExplorerAccountSynchronizer> {
        public:

            EthereumLikeBlockchainExplorerAccountSynchronizer(const std::shared_ptr<Services>& services,
                                                              const std::shared_ptr<EthereumLikeBlockchainExplorer>& explorer);

            void updateCurrentBlock(std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                                    const std::shared_ptr<api::ExecutionContext> &context) override;

            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr<SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

            std::shared_ptr<ProgressNotifier<Unit>> synchronize(const std::shared_ptr<EthereumLikeAccount>& account) override ;
            void reset(const std::shared_ptr<EthereumLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            bool isSynchronizing() const override;


        private:
            std::shared_ptr<EthereumBlockchainAccountSynchronizer> getSharedFromThis() override ;
            std::shared_ptr<api::ExecutionContext> getSynchronizerContext() override ;
        };
    }
}