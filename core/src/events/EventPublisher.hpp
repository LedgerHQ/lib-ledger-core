/*
 *
 * EventPublisher
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

#include <api/EventBus.hpp>
#include <api/EventPublisher.hpp>
#include <api/Event.hpp>
#include <api/EventCode.hpp>
#include <api/EventReceiver.hpp>
#include <unordered_set>
#include <memory>
#include <async/DedicatedContext.hpp>

namespace ledger {
    namespace core {
        class EventBus;
        typedef std::function<bool (const std::shared_ptr<api::Event>&)> EventFilter;
        class EventPublisher : public api::EventPublisher, public DedicatedContext, public std::enable_shared_from_this<EventPublisher> {
        public:
            EventPublisher(std::shared_ptr<api::ExecutionContext> context);
            std::shared_ptr<api::EventBus> getEventBus() override;
            void post(const std::shared_ptr<api::Event> &event) override;
            void postSticky(const std::shared_ptr<api::Event> &event, int32_t tag) override;
            void relay(const std::shared_ptr<api::EventBus> &bus) override;
            void setFilter(const EventFilter& filter);


        private:
            std::shared_ptr<EventBus> _bus;
            std::shared_ptr<api::EventReceiver> _receiver;
            EventFilter _filter;
        };
    }
}
