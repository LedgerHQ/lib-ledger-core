/*
 *
 * Future
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/01/2017.
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

#undef foreach

#include <exception>
#include <functional>
#include <list>
#include <memory>

#include <core/api/Error.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/async/Deffered.hpp>
#include <core/traits/CallbackTraits.hpp>
#include <core/traits/SharedPtrTraits.hpp>
#include <core/utils/Exception.hpp>
#include <core/utils/ImmediateExecutionContext.hpp>

namespace ledger {
    namespace core {
        template <typename T>
        class Future {
            typedef std::shared_ptr<api::ExecutionContext> Context;
        public:
            Future(const std::shared_ptr<Deffered<T>> &_defer) : _defer(_defer) {}
            Future(const Future<T>& future) {
                _defer = future._defer;
            }

            Future(Future<T>&& future) : _defer(future._defer) {}
            Future<T>& operator=(const Future<T>& future) {
                if (this != &future)
                    _defer = future._defer;
                return *this;
            }
            Future<T>& operator=(Future<T>&& future) {
                if (this != &future)
                    _defer = future._defer;
                return *this;
            }

            template <typename R>
            Future<R> map(const Context& context, std::function<R (const T&)> map) {
                auto defer = Future<R>::make_deffered();
                _defer->addCallback([defer, map] (const Try<T>& result) {
                    Try<R> r;
                    if (result.isSuccess()) {
                        r = Try<R>::from([map, result] () -> R {
                            return map(result.getValue());
                        });
                    } else {
                        r.fail(result.getFailure());
                    }
                    defer->setResult(r);
                }, context);
                return Future<R>(defer);
            }

            template <typename R>
            Future<std::shared_ptr<R>> mapPtr(const Context& context, std::function<std::shared_ptr<R> (const T&)> map) {
                return this->template map<std::shared_ptr<R>>(context, map);
            }

            template <typename R>
            Future<R> flatMap(const Context& context, std::function<Future<R> (const T&)> map) {
                auto deffer = Future<R>::make_deffered();
                _defer->addCallback([deffer, map, context] (const Try<T>& result) {
                    if (result.isSuccess()) {
                        Try<Future<R>> r;
                        r = Try<Future<R>>::from([deffer, map, result] () -> Future<R> {
                            return map(result.getValue());
                        });
                        if (r.isSuccess()) {
                            Future<R> future = r.getValue();
                            future.onComplete(context, [deffer] (const Try<R>& finalResult) {
                                deffer->setResult(finalResult);
                            });
                        } else {
                            Try<R> re;
                            re.fail(r.getFailure());
                            deffer->setResult(re);
                        }
                    } else {
                        Try<R> r;
                        r.fail(result.getFailure());
                        deffer->setResult(r);
                    }
                }, context);
                return Future<R>(deffer);
            }

            template <typename R>
            Future<std::shared_ptr<R>> flatMapPtr(const Context& context, std::function<Future<std::shared_ptr<R>> (const T&)> map) {
                return this->flatMap<std::shared_ptr<R>>(context, map);
            }

            Future<T> recover(const Context& context, std::function<T (const Exception&)> f) {
                auto deffer = Future<T>::make_deffered();
                _defer->addCallback([deffer, f] (const Try<T>& result) {
                    if (result.isFailure()) {
                        deffer->setResult(Try<T>::from([f, result] () {
                            return f(result.getFailure());
                        }));
                    } else {
                        deffer->setResult(result);
                    }
                }, context);
                return Future<T>(deffer);
            }

            Future<T> recoverWith(const Context& context, std::function<Future<T> (const Exception&)> f) {
                auto deffer = Future<T>::make_deffered();
                _defer->addCallback([deffer, f, context] (const Try<T>& result) {
                    if (result.isFailure()) {
                        auto future = Try<Future<T>>::from([f, result] () {
                            return f(result.getFailure());
                        });
                        if (future.isFailure()) {
                            Try<T> r;
                            r.fail(future.getFailure());
                            deffer->setResult(r);
                        } else {
                            Future<T> fut = future.getValue();
                            fut.onComplete(context, [deffer] (const Try<T>& finalResult) {
                                deffer->setResult(finalResult);
                            });
                        }
                    } else {
                        deffer->setResult(result);
                    }
                }, context);
                return Future<T>(deffer);
            }


            Future<T> fallback(const T& fallback) {
                return recover(ImmediateExecutionContext::INSTANCE, [fallback] (const Exception& ex) {
                   return fallback;
                });
            }
            Future<T> fallbackWith(Future<T> fallback) {
                return recoverWith(ImmediateExecutionContext::INSTANCE, [fallback] (const Exception& ex) {
                    return fallback;
                });
            }

            Future<T> filter(const Context& context, std::function<bool (const T&)> f) {
                return map<T>(context, [f] (const T& v) {
                    if (f(v)) {
                        return v;
                    } else {
                        throw Exception(api::ErrorCode::NO_SUCH_ELEMENT, "Value didn't pass the filter function.");
                    }
                });
            }

            void foreach(const Context& context, std::function<void (T&)> f) {
                _defer->addCallback([f] (const Try<T>& result) {
                    if (result.isSuccess()) {
                        T value = result.getValue();
                        f(value);
                    }
                }, context);
            }

            Option<Try<T>> getValue() const {
                return _defer->getValue();
            }

            bool isCompleted() const {
                return getValue().hasValue();
            }

            Future<Exception> failed() {
                auto deffer = Future<Exception>::make_deffered();
                _defer->addCallback([deffer] (const Try<T>& result) {
                    if (result.isFailure()) {
                        deffer->setResult(Try<Exception>(result.getFailure()));
                    } else {
                        Try<Exception> r;
                        r.fail(Exception(api::ErrorCode::FUTURE_WAS_SUCCESSFULL, "Future was successful but rejected cause of the failed projection."));
                        deffer->setResult(r);
                    }
                }, ImmediateExecutionContext::INSTANCE);
                return Future<Exception>(deffer);
            };

            void onComplete(const Context& context, std::function<void (const Try<T>&)> f) {
                _defer->addCallback(f, context);
            };

            template<typename Callback>
            typename std::enable_if<has_on_callback_method<Callback, void (T&)>::value, void>::type
            callback(const Context& context, std::shared_ptr<Callback> cb) {
                onComplete(context, [cb] (const Try<T>& result) {
                    cb->onCallback(result.getValue());
                });
            };

            template<typename Callback>
            typename std::enable_if<has_on_callback_method<Callback, void (const std::experimental::optional<T>&, const std::experimental::optional<api::Error>&)>::value, void>::type
            callback(const Context& context, std::shared_ptr<Callback> cb) {
                onComplete(context, [cb] (const Try<T>& result) {
                    if (result.isSuccess()) {
                        cb->onCallback(result.toOption().toOptional(), Option<api::Error>().toOptional());
                    } else {
                        cb->onCallback(Option<T>().toOptional(), Option<api::Error>(result.getFailure().toApiError()).toOptional());
                    }

                });
            };

            template<typename Callback>
            typename std::enable_if<is_shared_ptr<T>::value && has_on_callback_method<Callback, void (const T&, const std::experimental::optional<api::Error>&)>::value, void>::type
            callback(const Context& context, std::shared_ptr<Callback> cb) {
                onComplete(context, [cb] (const Try<T>& result) {
                    if (result.isSuccess()) {
                        cb->onCallback(result.getValue(), Option<api::Error>().toOptional());
                    } else {
                        cb->onCallback(nullptr, Option<api::Error>(result.getFailure().toApiError()).toOptional());
                    }

                });
            };

            static Future<T> successful(T value) {
                Promise<T> p;
                p.success(value);
                return p.getFuture();
            }

            static Future<T> failure(Exception&& exception) {
                Promise<T> p;
                p.failure(exception);
                return p.getFuture();
            }

            static Future<T> failure(const Exception& exception) {
                Promise<T> p;
                p.failure(exception);
                return p.getFuture();
            }

            static Future<T> async(const Context& context, std::function<T ()> f) {
                if (!context) {
                    throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Context has been released before async operation");
                }
                auto deffer = make_deffered();
                context->execute(make_runnable([deffer, f] () {
                    auto result = Try<T>::from([&] () -> T {
                        return f();
                    });
                    deffer->setResult(result);
                }));
                return Future<T>(deffer);
            }

            static Future<T> async(const Context& context, std::function<Future<T> ()> f) {
                if (!context) {
                    throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Context has been released before async operation");
                }
                auto deffer = make_deffered();
                context->execute(make_runnable([context, deffer, f] () {
                    auto result = Try<Future<T>>::from([&] () -> Future<T> {
                       return f();
                    });
                    if (result.isFailure()) {
                        deffer->setError(result.getFailure());
                    } else {
                        Future<T> f = result.getValue();
                        f.onComplete(context, [deffer] (const Try<T>& r) {
                            deffer->setResult(r);
                        });
                    }
                }));
                return Future<T>(deffer);
            }

            static std::shared_ptr<Deffered<T>> make_deffered() {
                auto d = new Deffered<T>();
                return std::shared_ptr<Deffered<T>>(d);
            };

        private:
            std::shared_ptr<Deffered<T>> _defer;
        };
        template <typename T>
        using FuturePtr = Future<std::shared_ptr<T>>;
    }
}
