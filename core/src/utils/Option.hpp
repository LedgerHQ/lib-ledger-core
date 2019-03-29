/*
 *
 * Option
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/01/2017.
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
#ifndef LEDGER_CORE_OPTION_HPP
#define LEDGER_CORE_OPTION_HPP

#include "utils/optional.hpp"
#include "Unit.hpp"
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

#undef foreach

namespace ledger {
    namespace core {
        template <typename T>
        class Option {
            static_assert(!std::is_reference<T>::value,
                          "Option is not compatible with references");
            static_assert(!std::is_abstract<T>::value,
                          "Option is not compatible with abstract types");
        public:
            typedef T BaseType;
            static const Option<T> NONE;

            Option() {};
            Option(const T& value) : _optional(value) {};
            Option(T&& value) : _optional(std::move(value)) {};

            Option(const Option<T>& option) : _optional(option._optional) {};
            Option(const std::ledger_exp::optional<T>& optional) : _optional(optional) {};
            Option<T>& operator=(const Option<T>& option) {
                if (this != &option) {
                    _optional = option._optional;
                }
                return *this;
            };

            Option<T>& operator=(Option<T>&& option) {
                if (this != &option) {
                    _optional = std::move(option._optional);
                }
                return *this;
            };

            Option<T>& operator=(const T& v) {
                _optional = std::ledger_exp::optional<T>(v);
                return *this;
            };

            Option<T>& operator=(T&& v) {
                _optional = std::ledger_exp::optional<T>(std::move(v));
                return *this;
            };

            Option<T>&operator=(T* v) {
                if (v == nullptr) {
                    _optional = std::ledger_exp::optional<T>(v);
                } else {
                    _optional = std::ledger_exp::optional<T>(*v);
                }
                return *this;
            };

            inline bool isEmpty() const {
              return !_optional;
            };

            inline bool hasValue() const {
                return !isEmpty();
            };

            inline bool nonEmpty() const {
                return !isEmpty();
            }

            T& operator*() & {return *_optional;};
            const T& operator*() const & { return *_optional;};
            T&& operator*() && {return std::move(*_optional);};
            const T&& operator*() const && { return  std::move(*_optional);};

            const T* operator->() const {
                return &_optional.value();
            }

            T* operator->() {
                return &_optional.value();
            }

            T& getValue() & {
                return _optional.value();
            }

            const T& getValue() const & {
                return _optional.value();
            }

            T&& getValue() && {
                return std::move(_optional.value());
            }

            const T&& getValue() const && {
                return std::move(_optional.value());
            }

            T getValueOr(const T& v) const& {
                if (isEmpty())
                    return v;
                else
                    return getValue();
            }

            T getOrElse(std::function<T ()> f) const {
                if (isEmpty())
                    return f();
                else
                    return getValue();
            }

            T getValueOr(T&& v) const & {
                if (isEmpty())
                    return std::forward<T>(v);
                return getValue();
            }

            T getValueOr(T&& v) && {
                if (isEmpty())
                    return std::move(v);
                return getValue();
            }

            std::ledger_exp::optional<T> toOptional() const {
                return _optional;
            };

            bool operator==(const T& v) const noexcept {
                return hasValue() && **this == v;
            }

            bool operator==(const Option<T>& v) const noexcept {
                return (isEmpty() == v.isEmpty()) || (hasValue() && v.hasValue() && **this == *v);
            }

            bool operator!=(const T& v) const noexcept {
                return !(*this == v);
            }

            bool operator!=(const Option<T>& v) const noexcept {
                return !(*this == v);
            }

            operator bool() const { return hasValue();};

            void foreach(std::function<void (const T& value)> f) const {
                if (hasValue()) {
                    f(_optional.value());
                }
            }

            bool forall(std::function<bool (const T& value)> f) const {
                if (hasValue()) {
                    return f(_optional.value());
                }
                return false;
            }

            template <typename A>
            Option<A> map(std::function<A (const T&)> f) const {
                if (isEmpty())
                    return Option<A>();
                return Option<A>(f(getValue()));
            };

            template <typename A>
            Option<A> flatMap(std::function<Option<A> (const T&)> f) const {
                if (isEmpty())
                    return Option<A>();
                return f(getValue());
            }

            std::vector<T> toVector() const {
                if (isEmpty())
                    return std::vector<T>();
                std::vector<T> out;
                out.push_back(getValue());
                return out;
            }

            template <typename A>
            Option<A> orElse(std::function<Option<A> ()> f) const {
                if (isEmpty()) {
                    return f();
                } else {
                    return Option<A>::NONE;
                }
            }

            operator std::ledger_exp::optional<T>() {
                return _optional;
            }

        private:
            std::ledger_exp::optional<T> _optional;
        };

        template <typename T>
        using OptionPtr = Option<std::shared_ptr<T>>;

        template <typename T>
        const Option<T> Option<T>::NONE;

        template <typename T>
        Option<std::shared_ptr<T>> make_option(const std::shared_ptr<T>& ptr) {
            if (ptr == nullptr)
                return Option<std::shared_ptr<T>>();
            else
                return Option<std::shared_ptr<T>>(ptr);
        }

    }
}

#endif //LEDGER_CORE_OPTION_HPP
