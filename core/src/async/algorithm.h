/*
 *
 * algorithm
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/07/2017.
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
#ifndef LEDGER_CORE_ALGORITHM_H
#define LEDGER_CORE_ALGORITHM_H

#include "Future.hpp"
#include <utils/ImmediateExecutionContext.hpp>

#include <tuple>

namespace ledger {
namespace core {
namespace async {

namespace internals {

template <class T>
Future<Unit> sequence_go(const std::shared_ptr<api::ExecutionContext>& context, int index, const std::vector<Future<T>>& futures, std::vector<T>* buffer)
{
    if (index >= futures.size()) {
        return Future<Unit>::successful(unit);
    } else {
        Future<T> fut = futures[index];
        return fut.template flatMap<Unit>(context, [context, buffer, index, futures](const T &r) -> Future<Unit> {
            buffer->push_back(r);
            return sequence_go(context, index + 1, futures, buffer);
        });
    }
}

} // namespace internals

template <class T>
Future<std::vector<T>> sequence(const std::shared_ptr<api::ExecutionContext>& context, const std::vector<Future<T>>& futures)
{
    auto buffer = new std::vector<T>();
    return internals::sequence_go<T>(context, 0, futures, buffer).template map<std::vector<T>>(context, [buffer] (const Unit&) -> std::vector<T> {
        auto res = *buffer;
        delete buffer;
        return res;
    });
}

namespace internals {

template<std::size_t i, std::size_t size, typename... T>
struct sequence_t
{
    static auto go(
        const std::shared_ptr<api::ExecutionContext>& context,
        std::tuple<Future<T>...>& tuple_futs,
        std::tuple<T...>* tuple) -> Future<Unit>
    {
        return std::get<i>(tuple_futs)
            .template flatMap<Unit>(context, [context, tuple_futs, tuple](const auto& r) mutable {
                std::get<i>(*tuple) = r;
                return sequence_t<i + 1, size, T...>::go(context, tuple_futs, tuple);
            });
    }
};

template<std::size_t size, typename... T>
struct sequence_t<size, size, T...>
{
    static auto go(
        const std::shared_ptr<api::ExecutionContext>& context,
        std::tuple<Future<T>...>& tuple_futs,
        std::tuple<T...>* tuple) -> Future<Unit>
    {
        return Future<Unit>::successful(unit);
    }
};

} // namespace internals

template<typename... T>
auto sequence(
    const std::shared_ptr<api::ExecutionContext>& context,
    std::tuple<Future<T>...>& tuple_futs) -> Future<std::tuple<T...>>
{
    auto tuple = new std::tuple<T...>();
    return internals::sequence_t<0, sizeof...(T), T...>::go(context, tuple_futs, tuple)
        .template map<std::tuple<T...>>(context, [tuple](const Unit&) {
            const auto res = *tuple;
            delete tuple;
            return res;
        });
}

} // namespace async
} // namespace core
} // namespace ledger

#define BEGIN_ASYNC_WHILE()

#define ASYNC_FOREACH()

#endif //LEDGER_CORE_ALGORITHM_H
