/*
 *
 * Event
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
#include "Event.hpp"

ledger::core::api::EventCode ledger::core::Event::getCode() {
    return _code;
}

std::shared_ptr<ledger::core::api::DynamicObject> ledger::core::Event::getPayload() {
    return _payload;
}

bool ledger::core::Event::isSticky() {
    return _sticky;
}

int32_t ledger::core::Event::getStickyTag() {
    return _tag;
}

ledger::core::Event::Event(ledger::core::api::EventCode code,
                           const std::shared_ptr<ledger::core::api::DynamicObject> &payload) {
    _code = code;
    _payload = payload;
    _sticky = false;
    if (payload) {
        std::static_pointer_cast<DynamicObject>(payload)->setReadOnly(true);
    }
}

void ledger::core::Event::setSticky(int32_t tag) {
    _tag = tag;
    _sticky = true;
}

std::shared_ptr<ledger::core::api::Event> ledger::core::make_event(ledger::core::api::EventCode code, const std::shared_ptr<ledger::core::api::DynamicObject>& payload) {
    return std::make_shared<ledger::core::Event>(code, payload);
}

std::shared_ptr<ledger::core::api::Event> ledger::core::api::Event::newInstance(EventCode code,
                                                                               const std::shared_ptr<DynamicObject> &payload) {
    return std::make_shared<ledger::core::Event>(code, payload);
}