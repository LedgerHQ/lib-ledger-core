/*
 *
 * MapLike
 * ledger-core
 *
 * Created by Pierre Pollastri on 02/03/2017.
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
#ifndef LEDGER_CORE_MAPLIKE_HPP
#define LEDGER_CORE_MAPLIKE_HPP

#include <unordered_map>
#include "../utils/Option.hpp"
#include "../utils/Try.hpp"
#include "../utils/Exception.hpp"
#include <fmt/format.h>

namespace ledger {
    namespace core {
        template<typename K, typename V, typename Container>
        class MapLike {
        public:
            MapLike() {}
            MapLike(std::initializer_list<std::pair<const K, V>> il) : _container(il) {};
            MapLike(const Container &base) {
                _container = base;
            };

            MapLike(const MapLike<K, V, Container> &map) {
                _container = map._container;
            }

            MapLike(MapLike<K, V, Container> &&map) {
                _container = std::move(map._container);
            }

            MapLike<K, V, Container>& operator=(const MapLike<K, V, Container>& map) {
                _container = map._container;
                return *this;
            };

            V &operator[](const K &key) {
                return _container[key];
            }

            V &at(const K &key) {
                try {
                    return _container.at(key);
                } catch (const std::out_of_range& e) {
                    throw Exception(api::ErrorCode::OUT_OF_RANGE, fmt::format("Key \"{}\" not found in map.", key));
                }
            }

            const V &at(const K &key) const {
                try {
                    return _container.at(key);
                } catch (const std::out_of_range& e) {
                    throw Exception(api::ErrorCode::OUT_OF_RANGE, fmt::format("Key \"{}\" not found in map.", key));
                }
            }

            Option<V> lift(const K &key) const {
                return Try<V>::from([this, &key] () {
                    return at(key);
                }).toOption();
            }

            bool contains(const K &key) const {
                return _container.find(key) != _container.end();
            }

            bool empty() const {
                return _container.empty();
            }

            bool nonEmpty() const {
                return !_container.empty();
            }

            size_t size() const {
                return _container.size();
            }

        private:
            Container _container;
        };

        template<typename K, typename V>
        using Map = MapLike<K, V, std::unordered_map<K, V>>;
    }
}
#endif //LEDGER_CORE_MAPLIKE_HPP
