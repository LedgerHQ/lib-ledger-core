/*
 *
 * callbacks
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/02/2017.
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
#ifndef LEDGER_CORE_CALLBACKS_HPP
#define LEDGER_CORE_CALLBACKS_HPP

#include <ledger/core/traits/callback_traits.hpp>
#include <functional>
#include <ledger/core/async/Promise.hpp>
#include <ledger/core/async/Future.hpp>
#include <ledger/core/utils/Exception.hpp>

using namespace ledger::core;

template <typename T, class Class, bool usesPtrInCallback>
class FutureCallback : public Class {

};

template <typename T, class Class>
class FutureCallback<T, Class, false> : public Class {
public:
    Future<T> getFuture() {
        return _promise.getFuture();
    };

    virtual
    void
    onCallback(const std::ledger_exp::optional<T>& result, const std::ledger_exp::optional<api::Error>& error) override {
        if (error) {
            _promise.failure(ledger::core::Exception(error.value().code, error.value().message));
        } else {
            _promise.success(result.value());
        }
    };

private:
    ledger::core::Promise<T> _promise;

};

template <typename T, class Class>
class FutureCallback<T, Class, true> : public Class {
public:
    Future<std::shared_ptr<T>> getFuture() {
        return _promise.getFuture();
    };

    virtual
    void
    onCallback(const std::shared_ptr<T>& result, const std::ledger_exp::optional<api::Error>& error) override {
        if (error) {
            _promise.failure(ledger::core::Exception(error.value().code, error.value().message));
        } else {
            _promise.success(result);
        }
    };

private:
    ledger::core::Promise<std::shared_ptr<T>> _promise;

};


template <typename T, class Class>
std::shared_ptr<FutureCallback<T, Class, has_on_callback_method<Class, void (const std::shared_ptr<T>&, const std::ledger_exp::optional<api::Error>&)>::value>>
make_api_callback() {
    return std::make_shared<FutureCallback<T, Class, has_on_callback_method<Class, void (const std::shared_ptr<T>&, const std::ledger_exp::optional<api::Error>&)>::value>>();
};

#endif //LEDGER_CORE_CALLBACKS_HPP
