/*
 *
 * TTLCache
 *
 * Created by El Khalil Bellakrid on 22/11/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <chrono>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include "Option.hpp"
/*
 * A really simple implementation of TTL cache
 */
namespace ledger {
    namespace core {
        template <typename K, typename V, typename Duration = std::chrono::seconds>
        class TTLCache {
        public:
            TTLCache(const Duration &ttl) : _ttl(ttl) {};

            Option<V> get(const K &key) {
                std::lock_guard<std::mutex> lock(_lock);
                auto it = _cache.find(key);
                if (it == _cache.end()) {
                    return Option<V>();
                } else if (getNowDurationSinceEpoch() - (*it).second.second > _ttl) {
                    // Clean the cache
                    _cache.erase(key);
                    return Option<V>();
                }
                return Option<V>((*it).second.first);
            };

            void put(const K &key, const V &value) {
                std::lock_guard<std::mutex> lock(_lock);
                _cache.insert(
                        { key,
                          { value,
                            getNowDurationSinceEpoch()
                          }
                        });
            };

            void erase(const K &key) {
                std::lock_guard<std::mutex> lock(_lock);
                _cache.erase(key);
            };

        private:
            Duration getNowDurationSinceEpoch() {
                return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now().time_since_epoch());
            }
            Duration _ttl;
            std::unordered_map<K, std::pair<V, Duration>> _cache;
            std::mutex _lock;
        };

    }
}