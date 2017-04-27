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
#include <collections/DynamicObject.hpp>
#include "EventPublisher.hpp"
#include "EventBus.hpp"
#include "Event.hpp"

namespace ledger {
    namespace core {

        class EventReceiver : public api::EventReceiver {
        public:
            EventReceiver(const std::shared_ptr<EventPublisher>& publisher) : _publisher(publisher) {

            };

            void onEvent(const std::shared_ptr<api::Event> &event) override {
                auto publisher = _publisher.lock();
                if (publisher) {
                    if (event->isSticky()) {
                        publisher->postSticky(event, event->getStickyTag());
                    } else {
                        publisher->post(event);
                    }
                }
            }

        private:
            std::weak_ptr<EventPublisher> _publisher;
        };

        EventPublisher::EventPublisher(std::shared_ptr<api::ExecutionContext> context) : DedicatedContext(context) {
            _bus = std::shared_ptr<EventBus>(new EventBus(context));
        }

        std::shared_ptr<api::EventBus> EventPublisher::getEventBus() {
            return _bus;
        }

        void EventPublisher::post(const std::shared_ptr<api::Event> &event) {
            auto ev = std::static_pointer_cast<Event>(event);
            if (ev->getPayload() != nullptr) {
                std::static_pointer_cast<DynamicObject>(ev->getPayload())->setReadOnly(true);
            }
            _bus->post(ev);
        }

        void EventPublisher::postSticky(const std::shared_ptr<api::Event> &event, int32_t tag) {
            auto ev = std::static_pointer_cast<Event>(event);
            if (ev->getPayload() != nullptr) {
                std::static_pointer_cast<DynamicObject>(ev->getPayload())->setReadOnly(true);
            }
            if (!ev->isSticky()) {
                ev->setSticky(tag);
            }
            _bus->post(ev);
        }

        void EventPublisher::relay(const std::shared_ptr<api::EventBus> &bus) {
            auto self = shared_from_this();
            run([=] () {
                if (!self->_receiver) {
                    self->_receiver = std::make_shared<EventReceiver>(self);
                }
                bus->subscribe(self->getContext(), self->_receiver);
            });
        }

        namespace api {
            std::shared_ptr<EventPublisher> EventPublisher::newInstance(const std::shared_ptr<ExecutionContext> &context) {
                return std::make_shared<ledger::core::EventReceiver>(context);
            }
        }
    }
}