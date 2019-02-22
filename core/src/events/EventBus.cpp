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

    std::weak_ptr<EventBus> weak_self(shared_from_this());

    async<Unit>([=] () {

        auto local_self = weak_self.lock();
        if (!local_self) {
            throw make_exception(api::ErrorCode::NULL_POINTER, "EventBus was released.");
        }
        auto local_context = context;
        auto local_receiver = receiver;

        for (auto& i : local_self->_subscribers) {
            auto& r = std::get<1>(i);
            if (r == local_receiver)
                return unit;
        }
        
        // Post all sticky event to the receiver
        //local_receiver is moved below when pushed in _subscribers, we need a new local_receiver
        //lambda passed to Future<Unit>::async
        std::weak_ptr<ledger::core::api::EventReceiver> local_weak_receiver(receiver);
        for (auto& event : local_self->_stickies) {
            auto lambda = [=] () {
                auto local_r = local_weak_receiver.lock();
                if (!local_r) {
                    throw make_exception(api::ErrorCode::NULL_POINTER, "Receiver was released.");
                }
                local_r->onEvent(event.second);
                return unit;
            };
            Future<Unit>::async(local_context, lambda);
        }

        local_self->_subscribers.push_back(std::make_tuple<std::shared_ptr<api::ExecutionContext>, std::shared_ptr<api::EventReceiver>>(std::move(local_context), std::move(local_receiver)));
        return unit;
    });
}

void ledger::core::EventBus::unsubscribe(const std::shared_ptr<ledger::core::api::EventReceiver> &receiver) {
    std::weak_ptr<EventBus> weak_self(shared_from_this());
    async<Unit>([=] () {
        auto local_self = weak_self.lock();
        if (!local_self) {
            throw make_exception(api::ErrorCode::NULL_POINTER, "EventBus was released.");
        }
        for (auto it = local_self->_subscribers.begin(); it != local_self->_subscribers.end(); it++) {
            auto& r = std::get<1>(*it);
            if (r == receiver) {
                local_self->_subscribers.erase(it);
                return unit;
            }
        }
        return unit;
    });
}

void ledger::core::EventBus::post(const std::shared_ptr<ledger::core::Event>& event) {
    std::weak_ptr<EventBus> weak_self(shared_from_this());
    run([=] () {
        auto local_self = weak_self.lock();
        if (!local_self) {
            throw make_exception(api::ErrorCode::NULL_POINTER, "EventBus was released.");
        }
        if (event->isSticky()) {
            local_self->_stickies[event->getStickyTag()] = event;
        }
        for (auto& subscriber : local_self->_subscribers) {
            auto& c = std::get<0>(subscriber);
            auto& r = std::get<1>(subscriber);

            std::weak_ptr<ledger::core::api::EventReceiver> weak_receiver(r);

            Future<Unit>::async(c, [=] () {
                auto local_receiver = weak_receiver.lock();
                if (!local_receiver) {
                    throw make_exception(api::ErrorCode::NULL_POINTER, "Receiver was released.");
                }
                local_receiver->onEvent(event);
                return unit;
            });
        }
    });
}

ledger::core::EventBus::EventBus(const std::shared_ptr<ledger::core::api::ExecutionContext> &context)
    : DedicatedContext(context) {

}
