/*
 *
 * LedgerApiEthereumLikeBlockchainObserver
 *
 * Created by El Khalil Bellakrid on 29/11/2018.
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


#ifndef LEDGER_CORE_LEDGERAPIETHEREUMLIKEBLOCKCHAINOBSERVER_H
#define LEDGER_CORE_LEDGERAPIETHEREUMLIKEBLOCKCHAINOBSERVER_H


#include "EthereumLikeBlockchainObserver.h"
#include <net/WebSocketClient.h>
#include <net/WebSocketConnection.h>
#include <wallet/common/observers/AbstractLedgerApiBlockchainObserver.h>
namespace ledger {
    namespace core {
        class LedgerApiEthereumLikeBlockchainObserver : public AbstractLedgerApiBlockchainObserver,
                                                        public EthereumLikeBlockchainObserver,
                                                        public std::enable_shared_from_this<LedgerApiEthereumLikeBlockchainObserver> {
        public:
            LedgerApiEthereumLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                                   const std::shared_ptr<WebSocketClient>& client,
                                                   const std::shared_ptr<api::DynamicObject>& configuration,
                                                   const std::shared_ptr<spdlog::logger>& logger,
                                                   const api::Currency &currency);

        protected:
            void onStart() override;

            void onStop() override;

        private:
            std::shared_ptr<spdlog::logger> logger() const override {
                return EthereumLikeBlockchainObserver::logger();
            };
            void connect() override ;
            void reconnect() override ;

            void onMessage(const std::string& message) override ;

        private:
            std::shared_ptr<WebSocketClient> _client;
            WebSocketEventHandler _handler;
        };
    }
}


#endif //LEDGER_CORE_LEDGERAPIETHEREUMLIKEBLOCKCHAINOBSERVER_H
