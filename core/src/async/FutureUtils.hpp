/*
*
* FutureUtils
* ledger-core
*
* Created by Andrii Korol on 22/11/2018.
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

#pragma once

#include <exception>
#include <memory>
#include <functional>
#include "Deffered.hpp"
#include "../api/ExecutionContext.hpp"
#include "../utils/Exception.hpp"
#include "../traits/callback_traits.hpp"
#include "../api/Error.hpp"
#include "Future.hpp"

namespace ledger {
    namespace core {

        template<typename T>
        struct Container {
            std::vector<T> result;
            std::mutex lock;
            uint32_t count;
        };
        
        template<typename T>
        Future<std::vector<T>> executeAll(const std::shared_ptr<api::ExecutionContext>& context, std::vector<Future<T>>& futures) {
            auto container = std::make_shared<Container<T>>();
            container->count = 0;
            container->result.resize(futures.size());
            auto deffered = std::make_shared<Deffered<std::vector<T>>>();
            for (int i = 0; i < futures.size(); ++i) {
                futures[i].onComplete(context, [container, deffered, i](const Try<T>& result) {
                    std::lock_guard<std::mutex> lock(container->lock);
                    if (deffered->hasValue())
                        return;
                    if (result.isSuccess()) {
                        container->result[i] = result.getValue();
                        container->count++;
                        if (container->count == container->result.size())
                            deffered->setValue(container->result);
                    }
                    else {
                        deffered->setError(result.getFailure());
                    }
                });
            }
            return Future<std::vector<T>>(deffered);
        }
    }
}
