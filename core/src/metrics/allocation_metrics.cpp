/*
 *
 * allocation_metrics.cpp
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

#include <api/AllocationMetrics.hpp>
#ifdef TARGET_JNI
#include <jni/jni/djinni_support.hpp>
#endif

#include <metrics/ManagedObject.hpp>

namespace ledger {
    namespace core {
        namespace api {

            std::unordered_map<std::string, int32_t> AllocationMetrics::getObjectAllocations() {
                std::unordered_map<std::string, int32_t> allocations;
                for (const auto &alloc : AllocationMap::getInstance()->getAllocations()) {
                    allocations[alloc.first.name()] = alloc.second;
                }
                return allocations;
            }

        } // namespace api
    }     // namespace core
} // namespace ledger
