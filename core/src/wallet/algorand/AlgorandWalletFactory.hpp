/*
 *
 * AlgorandWalletFactory
 *
 * Created by Hakim Aammar on 11/05/2020.
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

#ifndef LEDGER_CORE_ALGORANDWALLETFACTORY_H
#define LEDGER_CORE_ALGORANDWALLETFACTORY_H

#include "AlgorandBlockchainExplorer.hpp"
#include "AlgorandBlockchainObserver.hpp"
#include "AlgorandAccountSynchronizer.hpp"

#include <wallet/common/AbstractWalletFactory.hpp>
//#include <core/Services.hpp>
#include <wallet/pool/WalletPool.hpp> // V1 for core/Services.hpp ?

namespace ledger {
namespace core {
namespace algorand {

    using AlgorandAccountSynchronizerFactory = std::function<std::shared_ptr<AccountSynchronizer>()>;

    class WalletFactory : public AbstractWalletFactory {
    public:
        WalletFactory(const api::Currency &currency, const std::shared_ptr<WalletPool>& pool);

        std::shared_ptr<AbstractWallet> build(const WalletDatabaseEntry &entry) override;

    private:
        std::shared_ptr<BlockchainExplorer>
        getExplorer(const std::string &currencyName, const std::shared_ptr<api::DynamicObject> &configuration);

        std::shared_ptr<BlockchainObserver>
        getObserver(const std::string &currencyName, const std::shared_ptr<api::DynamicObject> &configuration);

    private:
        std::list<std::weak_ptr<BlockchainExplorer>> _runningExplorers;
        std::list<std::weak_ptr<BlockchainObserver>> _runningObservers;
    };

} // namespace algorand
} // namespace core
} // namespace ledeger

#endif // LEDGER_CORE_ALGORANDWALLETFACTORY_H