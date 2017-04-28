/*
 *
 * BitcoinLikeBlockchainObserver
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/04/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#ifndef LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP
#define LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP

#include <src/events/EventPublisher.hpp>
#include <src/async/DedicatedContext.hpp>

namespace ledger {
    namespace core {
        class BitcoinLikeBlockchainObserver : public DedicatedContext {
        public:
            BitcoinLikeBlockchainObserver(const std::shared_ptr<api::ExecutionContext>& context) : DedicatedContext(context) {
                _publisher = std::make_shared<EventPublisher>(context);
            };
            virtual void start() = 0;
            virtual void stop() = 0;
            virtual bool isObserving() const = 0;
            std::shared_ptr<EventBus> getEventBus() const {
                return _publisher->getEventBus();
            };

        protected:
            virtual std::shared_ptr<EventPublisher> getEventPublisher() const {
                return _publisher;
            }
        private:
            std::shared_ptr<EventPublisher> _publisher;
        };
    }
}

#endif //LEDGER_CORE_BITCOINLIKEBLOCKCHAINOBSERVER_HPP
