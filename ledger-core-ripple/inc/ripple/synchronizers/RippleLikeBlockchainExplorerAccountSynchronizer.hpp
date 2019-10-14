/*
 *
 * RippleLikeBlockchainExplorerAccountSynchronizer
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#pragma once

#include <core/Services.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/events/ProgressNotifier.hpp>
#include <core/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.hpp>

#include <ripple/explorers/RippleLikeBlockchainExplorer.hpp>
#include <ripple/keychains/RippleLikeKeychain.hpp>
#include <ripple/synchronizers/RippleLikeAccountSynchronizer.hpp>

namespace ledger {
    namespace core {
        class RippleLikeAccount;

        using RippleBlockchainAccountSynchronizer = AbstractBlockchainExplorerAccountSynchronizer<RippleLikeAccount, RippleLikeAddress, RippleLikeKeychain, RippleLikeBlockchainExplorer>;

        class RippleLikeBlockchainExplorerAccountSynchronizer : public RippleBlockchainAccountSynchronizer,
                                                                public RippleLikeAccountSynchronizer,
                                                                public DedicatedContext,
                                                                public std::enable_shared_from_this<RippleLikeBlockchainExplorerAccountSynchronizer> {
        public:

            RippleLikeBlockchainExplorerAccountSynchronizer(const std::shared_ptr<Services> &services,
                                                            const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer);

            void updateCurrentBlock(
                    std::shared_ptr<AbstractBlockchainExplorerAccountSynchronizer::SynchronizationBuddy> &buddy,
                    const std::shared_ptr<api::ExecutionContext> &context) override;

            void updateTransactionsToDrop(soci::session &sql,
                                          std::shared_ptr<SynchronizationBuddy> &buddy,
                                          const std::string &accountUid) override;

            std::shared_ptr<ProgressNotifier<Unit>>
            synchronize(const std::shared_ptr<RippleLikeAccount> &account) override;

            void reset(const std::shared_ptr<RippleLikeAccount> &account,
                       const std::chrono::system_clock::time_point &toDate) override;

            bool isSynchronizing() const override;


        private:
            std::shared_ptr<RippleBlockchainAccountSynchronizer> getSharedFromThis() override;

            std::shared_ptr<api::ExecutionContext> getSynchronizerContext() override;
        };
    }
}
