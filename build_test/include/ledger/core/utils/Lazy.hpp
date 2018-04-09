/*
 *
 * Lazy
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/02/2017.
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
#ifndef LEDGER_CORE_LAZY_HPP
#define LEDGER_CORE_LAZY_HPP

#include <functional>
#include "Option.hpp"
#include "Exception.hpp"

namespace ledger {
    namespace core {
        template <typename T>
        class Lazy {
        public:

            Lazy() {
                _value = nullptr;
                _creator = [] () -> std::shared_ptr<T> {
                    throw Exception(api::ErrorCode::RUNTIME_ERROR, "Empty lazy");
                };
            }

            Lazy(std::function<std::shared_ptr<T> ()> creator) {
                _creator = creator;
                _value = nullptr;
            };

            Lazy(const Lazy<T>& value) {
                _creator = value._creator;
                _value = value._value;
            };

            Lazy& operator=(const Lazy<T>& value) {
                _creator = value._creator;
                _value = value._value;
            };

            operator T() {
                return get();
            }
            T* operator->() {
                return &get();
            }
            T&operator*() {
                return get();
            }

            T& get() {
                if (_value != nullptr)
                    return *_value.get();
                else {
                    _value = _creator();
                    return *_value.get();
                }
            };

            template <typename... Args>
            static Lazy<T> make_lazy(Args... args) {
                return Lazy<T>(
                [=] () {
                    return std::make_shared<T>(args...);
                });
            }

        private:
            std::function<std::shared_ptr<T> ()> _creator;
            std::shared_ptr<T> _value;
        };
    }
}


#endif //LEDGER_CORE_LAZY_HPP
