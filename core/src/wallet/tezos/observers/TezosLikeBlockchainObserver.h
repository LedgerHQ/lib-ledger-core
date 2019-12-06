/*
 *
 * TezosLikeBlockchainObserver
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


#ifndef LEDGER_CORE_TEZOSLIKEBLOCKCHAINOBSERVER_H
#define LEDGER_CORE_TEZOSLIKEBLOCKCHAINOBSERVER_H

#include <string>
#include <list>

#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>

#include <utils/ConfigurationMatchable.h>
#include <async/DedicatedContext.hpp>

#include <api/ExecutionContext.hpp>
#include <api/Currency.hpp>
#include <api/DynamicObject.hpp>

#include <net/WebSocketClient.h>
#include <net/WebSocketConnection.h>

#include <spdlog/logger.h>
#include <wallet/common/observers/AbstractBlockchainObserver.h>
#include <wallet/common/observers/AbstractLedgerApiBlockchainObserver.h>

namespace ledger {
    namespace core {
        class TezosLikeAccount;

        using TezosBlockchainObserver = AbstractBlockchainObserver<TezosLikeAccount, TezosLikeBlockchainExplorerTransaction, TezosLikeBlockchainExplorer::Block>;

        class TezosLikeBlockchainObserver : public TezosBlockchainObserver,
                                            public AbstractLedgerApiBlockchainObserver,
                                            public DedicatedContext,
                                            public ConfigurationMatchable,
                                            public std::enable_shared_from_this<TezosLikeBlockchainObserver> {
        public:
            TezosLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                        const std::shared_ptr<api::DynamicObject> &configuration,
                                        const std::shared_ptr<spdlog::logger> &logger,
                                        const api::Currency &currency,
                                        const std::vector<std::string> &matchableKeys);

            TezosLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                        const std::shared_ptr<WebSocketClient> &client,
                                        const std::shared_ptr<api::DynamicObject> &configuration,
                                        const std::shared_ptr<spdlog::logger> &logger,
                                        const api::Currency &currency);


        protected:
            void putTransaction(const TezosLikeBlockchainExplorerTransaction &tx) override;

            void putBlock(const TezosLikeBlockchainExplorer::Block &block) override;

            const api::Currency &getCurrency() const {
                return _currency;
            };

            std::shared_ptr<api::DynamicObject> getConfiguration() const {
                return _configuration;
            };

        private:
            api::Currency _currency;
            std::shared_ptr<api::DynamicObject> _configuration;


        protected:
            void onStart() override;

            void onStop() override;

        private:
            void connect() override;

            void reconnect() override;

            void onMessage(const std::string &message) override;

        private:
            std::shared_ptr<spdlog::logger> logger() const override;

            std::shared_ptr<WebSocketClient> _client;
            WebSocketEventHandler _handler;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKEBLOCKCHAINOBSERVER_H
