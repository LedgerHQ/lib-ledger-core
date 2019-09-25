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

#pragma once

#include <tuple>
#include <unordered_map>
#include <list>

#include <core/events/EventPublisher.hpp>
#include <core/events/Event.hpp>

namespace ledger {
    namespace core {
        class EventBus : public api::EventBus, public DedicatedContext, public std::enable_shared_from_this<EventBus> {
        public:
            void subscribe(const std::shared_ptr<api::ExecutionContext> &context,
                           const std::shared_ptr<api::EventReceiver> &receiver) override;
            void unsubscribe(const std::shared_ptr<api::EventReceiver> &receiver) override;

        private:
            explicit EventBus(const std::shared_ptr<api::ExecutionContext>& context);
            friend class EventPublisher;
            void post(const std::shared_ptr<Event>& event);


        private:
            using SubscribersList = std::list<std::tuple<std::shared_ptr<api::ExecutionContext>, std::shared_ptr<api::EventReceiver>>>;
            SubscribersList _subscribers;
            using StickiesMap = std::unordered_map<int32_t, std::shared_ptr<Event>>;
            StickiesMap _stickies;
        };
    }
}