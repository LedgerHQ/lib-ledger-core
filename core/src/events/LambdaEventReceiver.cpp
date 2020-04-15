/*
 *
 * LambdaEventReceiver
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
#include "LambdaEventReceiver.hpp"
#include <utils/Unit.hpp>
#include <async/Promise.hpp>
#include <api/DynamicObject.hpp>

namespace ledger {
    namespace core {

        std::shared_ptr<LambdaEventReceiver> make_receiver(std::function<void (const std::shared_ptr<api::Event> &)> f) {
            return std::make_shared<LambdaEventReceiver>(f);
        }

        LambdaEventReceiver::LambdaEventReceiver(std::function<void(const std::shared_ptr<api::Event> &)> f) {
            _function = f;
        }

        void LambdaEventReceiver::onEvent(const std::shared_ptr<api::Event> &event) {
            _function(event);
        }

        std::shared_ptr<LambdaEventReceiver> make_promise_receiver(
                Promise<Unit>& promise,
                const std::vector<api::EventCode> &successCodes,
                const std::vector<api::EventCode> &failureCodes
        ) {
            return make_receiver([=] (const std::shared_ptr<api::Event> &event) mutable {
                for (const auto& code : successCodes) {
                    if (code == event->getCode()) {
                        promise.success(unit);
                        return;
                    }
                }
                for (const auto& code : failureCodes) {
                    if (code == event->getCode()) {
                        promise.failure(make_exception(api::ErrorCode::RUNTIME_ERROR, event->getPayload()->dump()));
                        return;
                    }
                }
            });
        }

    }
}