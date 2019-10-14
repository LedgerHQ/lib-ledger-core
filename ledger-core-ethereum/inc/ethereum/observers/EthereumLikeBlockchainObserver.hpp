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

#pragma once

#include <string>
#include <list>

#include <spdlog/logger.h>

#include <core/api/ExecutionContext.hpp>
#include <core/api/Currency.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/net/WebSocketClient.hpp>
#include <core/net/WebSocketConnection.hpp>
#include <core/utils/ConfigurationMatchable.hpp>
#include <core/observers/AbstractBlockchainObserver.hpp>

#include <ethereum/explorers/EthereumLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        class EthereumLikeAccount;
        using EthereumBlockchainObserver = AbstractBlockchainObserver<EthereumLikeAccount, EthereumLikeBlockchainExplorerTransaction, EthereumLikeBlockchainExplorer::Block>;
        class EthereumLikeBlockchainObserver : public EthereumBlockchainObserver,
                                               public DedicatedContext,
                                               public ConfigurationMatchable {
        public:
            EthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                           const std::shared_ptr<api::DynamicObject> &configuration,
                                           const std::shared_ptr<spdlog::logger> &logger,
                                           const api::Currency &currency,
                                           const std::vector<std::string> &matchableKeys);


        protected:
            void putTransaction(const EthereumLikeBlockchainExplorerTransaction &tx) override;

            void putBlock(const EthereumLikeBlockchainExplorer::Block &block) override;

            const api::Currency &getCurrency() const {
                return _currency;
            };

            std::shared_ptr<api::DynamicObject> getConfiguration() const {
                return _configuration;
            };

        private:
            api::Currency _currency;
            std::shared_ptr<api::DynamicObject> _configuration;
        };
    }
}