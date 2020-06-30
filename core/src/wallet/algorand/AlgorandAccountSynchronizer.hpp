/*
 * AlgorandAccountSynchronizer
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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


#ifndef LEDGER_CORE_ALGORANDACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_ALGORANDACCOUNTSYNCHRONIZER_H

#include "AlgorandAddress.hpp"
#include "AlgorandBlockchainExplorer.hpp"

#include <wallet/pool/WalletPool.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <events/ProgressNotifier.h>

#include <mutex>
#include <string>
#include <map>
#include <vector>

namespace ledger {
namespace core {
namespace algorand {

    struct SavedState {
        uint64_t round;

        SavedState() : round(0) {}

        template<class Archive>
        void serialize(Archive & archive) {
            archive(round); // Serialize things by passing them to the archive
        };
    };

    class Account;

    class AccountSynchronizer : public DedicatedContext,
                                public std::enable_shared_from_this<AccountSynchronizer> {

    public:

        AccountSynchronizer(const std::shared_ptr<WalletPool>& pool,
                            const std::shared_ptr<BlockchainExplorer> & explorer);

        std::shared_ptr<ProgressNotifier<Unit>> synchronizeAccount(const std::shared_ptr<Account> & account);

    private:

        Future<Unit> performSynchronization(const std::shared_ptr<Account> & account);

        Future<bool> synchronizeBatch(const std::shared_ptr<Account> & account,
                                      const bool hadTransactions,
                                      const Option<uint64_t> & lowestRound = Option<uint64_t>(),
                                      const Option<uint64_t> & highestRound = Option<uint64_t>());

        Future<Unit> updateLatestBlock(const std::shared_ptr<api::ExecutionContext> &context);

        std::shared_ptr<Account> _account;
        std::shared_ptr<BlockchainExplorer> _explorer;
        std::shared_ptr<Preferences> _internalPreferences;
        std::shared_ptr<ProgressNotifier<Unit>> _notifier;
        std::mutex _lock;

    };

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDACCOUNTSYNCHRONIZER_H
