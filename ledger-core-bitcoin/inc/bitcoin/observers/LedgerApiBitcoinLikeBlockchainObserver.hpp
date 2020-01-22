/*
 *
 * LedgerApiBitcoinLikeBlockchainObserver.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <core/net/WebSocketClient.hpp>
#include <core/net/WebSocketConnection.hpp>
#include <core/observers/AbstractLedgerApiBlockchainObserver.hpp>

#include <bitcoin/observers/BitcoinLikeBlockchainObserver.hpp>

namespace ledger {
    namespace core {
        class LedgerApiBitcoinLikeBlockchainObserver : public AbstractLedgerApiBlockchainObserver,
                                                       public BitcoinLikeBlockchainObserver,
                                                       public std::enable_shared_from_this<LedgerApiBitcoinLikeBlockchainObserver> {
        public:
            LedgerApiBitcoinLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext> &context,
                                                   const std::shared_ptr<WebSocketClient>& client,
                                                   const std::shared_ptr<api::DynamicObject>& configuration,
                                                   const std::shared_ptr<spdlog::logger>& logger,
                                                   const api::Currency &currency);

        protected:
            void onStop() override;

            void onStart() override;


        private:
            std::shared_ptr<spdlog::logger> logger() const override {
                return BitcoinLikeBlockchainObserver::logger();
            };
            void connect() override ;
            void reconnect() override ;
            void onMessage(const std::string& message) override ;

        private:
            std::shared_ptr<WebSocketClient> _client;
            //std::shared_ptr<WebSocketConnection> _socket;
            WebSocketEventHandler _handler;
            //int32_t _attempt;
            //std::string _url;
        };
    }
}