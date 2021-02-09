/*
 *
 * RippleLikeAccountSynchronizer
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

#ifndef LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H
#define LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H

#include <async/DedicatedContext.hpp>
#include <async/Future.hpp>
#include <events/ProgressNotifier.h>
#include <wallet/ripple/explorers/RippleLikeBlockchainExplorer.h>
#include <wallet/pool/WalletPool.hpp>

#include <chrono>
#include <memory>
#include <mutex>

namespace ledger {
namespace core {

class RippleLikeAccount;

class RippleLikeAccountSynchronizer : public DedicatedContext
{
public:
    struct SavedState {
        uint64_t blockHeight{0};
        std::string blockHash{};

        template<typename Archive>
        void serialize(Archive& archive) {
            archive(blockHeight, blockHash);
        }
    };

    struct Result {
        uint64_t lastBlockHeight{0};
        uint32_t newOperations{0};
    };

    RippleLikeAccountSynchronizer(
        const std::shared_ptr<WalletPool>& pool,
        const std::shared_ptr<RippleLikeBlockchainExplorer>& explorer);

    std::shared_ptr<ProgressNotifier<Result>>
    synchronize(const std::shared_ptr<RippleLikeAccount>& account);

private:
    Future<Result> performSynchronization(
        const std::shared_ptr<RippleLikeAccount>& account) const;

    Future<uint32_t> synchronizeTransactions(
        const std::shared_ptr<RippleLikeAccount>& account, SavedState& state, uint32_t nbTxns = 0) const;

    void updateCurrentBlock(const std::shared_ptr<RippleLikeAccount>& account) const;

private:
    std::shared_ptr<RippleLikeBlockchainExplorer> _explorer;
    std::shared_ptr<ProgressNotifier<Result>> _notifier;
    std::shared_ptr<spdlog::logger> _logger;
    std::chrono::system_clock::time_point _start;
    std::mutex _m;
};

} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_RIPPLELIKEACCOUNTSYNCHRONIZER_H
