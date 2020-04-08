/*
 *
 * EthereumLikeBlockchainObserver
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


#include "EthereumLikeBlockchainObserver.h"
#include <api/Configuration.hpp>
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <math/Fibonacci.h>
#include <utils/JSONUtils.h>
namespace ledger {
    namespace core {

        EthereumLikeBlockchainObserver::EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext>& context,
                                                                       const std::shared_ptr<api::DynamicObject>& configuration,
                                                                       const std::shared_ptr<spdlog::logger>& logger,
                                                                       const api::Currency& currency,
                                                                       const std::vector<std::string>& matchableKeys) :
                                                                       DedicatedContext(context), ConfigurationMatchable(matchableKeys) {

            _currency = currency;
            _configuration = configuration;
            setConfiguration(configuration);
            setLogger(logger);
        }


        void EthereumLikeBlockchainObserver::putTransaction(const EthereumLikeBlockchainExplorerTransaction &tx) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, tx] () {
                    bool shouldEmitNow = false;
                    {
                        soci::session sql(account->getWallet()->getDatabase()->getPool());
                        soci::transaction tr(sql);
                        shouldEmitNow = account->putTransaction(sql, tx) != EthereumLikeAccount::FLAG_TRANSACTION_IGNORED;
                        tr.commit();
                    }
                    if (shouldEmitNow) {
                        account->emitEventsNow();
                    }
                });
            }
        }

        void EthereumLikeBlockchainObserver::putBlock(const EthereumLikeBlockchainExplorer::Block &block) {
            std::lock_guard<std::mutex> lock(_lock);
            for (const auto& account : _accounts) {
                account->run([account, block] () {
                    bool shouldEmitNow = false;
                    {
                        soci::session sql(account->getWallet()->getDatabase()->getPool());
                        soci::transaction tr(sql);
                        shouldEmitNow = account->putBlock(sql, block);
                        tr.commit();
                    }
                    if (shouldEmitNow) {
                        account->emitEventsNow();
                    }
                });
            }
        }


    }
}