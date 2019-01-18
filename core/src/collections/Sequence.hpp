/*
 *
 * Sequence
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/03/2017.
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
#ifndef LEDGER_CORE_SEQUENCE_HPP
#define LEDGER_CORE_SEQUENCE_HPP

#include <vector>
#include <list>
#include "../utils/Exception.hpp"
#include <iterator>

namespace ledger {
    namespace core {
        template <typename T, typename Container>
        class Sequence {
        public:
            Sequence() {

            }

            Sequence(const Container& container) : _container(container) {

            }

            /// Type-safe indexed getter.
            optional<T&> get(size_t index) {
                if (index < size()) {
                    return this->operator[](index);
                } else {
                    return optional<T&>();
                }
            }

            /// Type-safe indexed getter.
            optional<const T&> get(size_t index) const {
                if (index < size()) {
                    return this->operator[](index);
                } else {
                    return optional<const T&>();
                }
            }


            T& operator[](size_t index) {
                return _container[index];
            }

            const T& operator[](size_t index) const {
                return _container[index];
            }

            size_t size() const {
                return _container.size();
            }

            void remove(size_t index) {
                if (index > size())
                    throw Exception(api::ErrorCode::OUT_OF_RANGE, fmt::format("Index {} is out of range ({})", index, size()));
                auto it = _container.begin();
                std::advance(it, index);
                _container.erase(it);
            }

            Sequence<T, Container>& operator+=(const T& v) {
                _container.push_back(v);
                return *this;
            }

            Container& getContainer() {
                return _container;
            }

            const Container& getContainer() const {
                return _container;
            }

            template<typename Result>
            Option<Result> join(std::function<Result (const T&, const Option<Result>&)> f) const {
                Option<T> carry;
                for (auto& item : _container) {
                   carry = Option<T>(f(item, carry));
                }
                return carry;
            }

        private:
            Container _container;
        };

        template <typename T>
        using Array = Sequence<T, std::vector<T>>;

        template <typename T>
        using List = Sequence<T, std::vector<T>>;

    }
}

#endif //LEDGER_CORE_SEQUENCE_HPP
