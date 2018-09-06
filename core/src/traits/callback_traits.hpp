/*
 *
 * callback_traits
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/02/2017.
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
#ifndef LEDGER_CORE_CALLBACK_TRAITS_HPP
#define LEDGER_CORE_CALLBACK_TRAITS_HPP

#include <cstddef>
#include <type_traits>

namespace ledger {
    namespace core {

        template<typename, typename T>
        struct has_on_callback_method {
            static_assert(
                    std::integral_constant<T, false>::value,
                    "Second template parameter needs to be of function type.");
        };

// specialization that does the checking

        template<typename C, typename Ret, typename... Args>
        struct has_on_callback_method<C, Ret(Args...)> {
        private:
            template<typename T>
            static constexpr auto check(T*)
            -> typename
            std::is_same<
                    decltype( std::declval<T>().onCallback( std::declval<Args>()... ) ),
                    Ret    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            >::type;  // attempt to call it and see if the return type is correct

            template<typename>
            static constexpr std::false_type check(...) { return std::false_type();};

            typedef decltype(check<C>(0)) type;

        public:
            static constexpr bool value = type::value;
        };
    }
}

#endif //LEDGER_CORE_CALLBACK_TRAITS_HPP
