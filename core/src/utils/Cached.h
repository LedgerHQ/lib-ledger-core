/*
 *
 * ledger-core
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

#include <functional>
#include <tuple>
#include <unordered_map>
namespace hash_tuple {

    template <typename TT>
    struct hash {
        size_t
        operator()(TT const &tt) const {
            return std::hash<TT>()(tt);
        }
    };

    namespace {
        template <class T>
        inline void hash_combine(std::size_t &seed, T const &v) {
            seed ^= hash_tuple::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2); // NOLINT(hicpp-signed-bitwise)
        }

        // Recursive template code derived from Matthieu M.
        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl {
            static void apply(size_t &seed, Tuple const &tuple) {
                HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
                hash_combine(seed, std::get<Index>(tuple));
            }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple, 0> {
            static void apply(size_t &seed, Tuple const &tuple) {
                hash_combine(seed, std::get<0>(tuple));
            }
        };
    } // namespace

    template <typename... TT>
    struct hash<std::tuple<TT...>> {
        size_t
        operator()(std::tuple<TT...> const &tt) const {
            size_t seed = 0;
            HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
            return seed;
        }
    };

} // namespace hash_tuple
namespace ledger::core::utils {

            template <typename Function>
            struct function_traits
                : public function_traits<decltype(&Function::operator())> {};

            template <typename ClassType, typename ReturnType, typename... Args>
            struct function_traits<ReturnType (ClassType::*)(Args...) const> {
                using pointer  = ReturnType (*)(Args...);
                using function = std::function<ReturnType(Args...)>;
            };

            template <typename Function>
            typename function_traits<Function>::function
            to_function(Function lambda) {
                return typename function_traits<Function>::function(lambda);
            }

            template <typename R, typename... Args>
            using cache_type = std::unordered_map<std::tuple<Args...>, R, hash_tuple::hash<std::tuple<Args...>>>;

            template <typename R, typename... Args>
            std::function<R(Args...)> cached(cache_type<R, Args...> &cache, std::function<R(Args...)> f) {
                return [f, &cache](Args... args) {
                    auto key = make_tuple(args...);
                    if (cache.count(key) > 0) {
                        return cache[key];
                    }
                    R result = f(std::forward<Args>(args)...);
                    cache.insert(std::pair<decltype(key), R>(key, result));
                    return result;
                };
            }
        } // namespace utils
    }     // namespace core
} // namespace ledger
