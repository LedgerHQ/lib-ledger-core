/*
 *
 * EventBus
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/04/2017.
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
#include "EventBus.hpp"

void ledger::core::EventBus::subscribe(const std::shared_ptr<ledger::core::api::ExecutionContext> &context,
                                       const std::shared_ptr<ledger::core::api::EventReceiver> &receiver) {
    auto self = shared_from_this();
    async<Unit>([=] () {
        for (auto& i : self->_subscribers) {
            auto& r = std::get<1>(i);
            if (r == receiver)
                return unit;
        }
        std::shared_ptr<ledger::core::api::ExecutionContext> c = context;
        std::shared_ptr<ledger::core::api::EventReceiver> r = receiver;
        self->_subscribers.push_back(std::make_tuple<std::shared_ptr<api::ExecutionContext>, std::shared_ptr<api::EventReceiver>>(std::move(c), std::move(r)));
        // Post all sticky event to the receiver
        for (auto& event : self->_stickies) {
            Future<Unit>::async(context, [=] () {
                receiver->onEvent(event.second);
                return unit;
            });
        }
        return unit;
    });
}

void ledger::core::EventBus::unsubscribe(const std::shared_ptr<ledger::core::api::EventReceiver> &receiver) {
    auto self = shared_from_this();
    async<Unit>([=] () {
        for (auto it = _subscribers.begin(); it != _subscribers.end(); it++) {
            auto& r = std::get<1>(*it);
            if (r == receiver) {
                self->_subscribers.erase(it);
                return unit;
            }
        }
        return unit;
    });
}

void ledger::core::EventBus::post(const std::shared_ptr<ledger::core::Event> event) {
    auto self = shared_from_this();
    run([=] () {
        if (event->isSticky()) {
            self->_stickies[event->getStickyTag()] = event;
        }
        for (auto& subscriber : self->_subscribers) {
            auto& c = std::get<0>(subscriber);
            auto& r = std::get<1>(subscriber);
            Future<Unit>::async(c, [=] () {
                r->onEvent(event);
                return unit;
            });
        }
    });
}

ledger::core::EventBus::EventBus(const std::shared_ptr<ledger::core::api::ExecutionContext> &context)
    : DedicatedContext(context) {

}
