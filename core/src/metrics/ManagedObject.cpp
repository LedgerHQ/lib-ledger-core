/*
 *
 * ManagedObject.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/11/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "ManagedObject.hpp"

namespace ledger {
    namespace core {

        void AllocationMap::increment(std::type_index idx) {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_allocations.find(idx) == _allocations.end()) {
                _allocations[idx] = 1;
            } else {
                _allocations[idx] += 1;
            }
        }

        void AllocationMap::decrement(std::type_index idx) {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_allocations.find(idx) != _allocations.end()) {
                _allocations[idx] -= 1;
            }
        }

        std::unordered_map<std::type_index, int> AllocationMap::getAllocations() {
            std::unique_lock<std::mutex> lock(_mutex);
            return _allocations;
        }

        std::shared_ptr<AllocationMap> AllocationMap::getInstance() {
            static std::shared_ptr<AllocationMap> instance = std::make_shared<AllocationMap>();
            return instance;
        }
    }
}