/*
 *
 * ManagedObject.hpp
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

#ifndef LEDGER_CORE_MANAGEDOBJECT_HPP
#define LEDGER_CORE_MANAGEDOBJECT_HPP

#include <unordered_map>
#include <mutex>
#include <typeindex>
#include <memory>


namespace ledger {
    namespace core {

        class AllocationMap {
        public:
            void increment(std::type_index idx);
            void decrement(std::type_index idx);
            std::unordered_map<std::type_index, int> getAllocations();

            static std::shared_ptr<AllocationMap> getInstance();

        private:
            std::mutex _mutex;
            std::unordered_map<std::type_index, int> _allocations;
        };

        template <typename Type>
        class ManagedObject {
        public:
            ManagedObject() {
                AllocationMap::getInstance()->increment(typeid(Type));
            }

            ManagedObject(const ManagedObject& cpy) {
                AllocationMap::getInstance()->increment(typeid(Type));
            }

            virtual ~ManagedObject() {
                AllocationMap::getInstance()->decrement(typeid(Type));
            }
        };


    }
}

#endif //LEDGER_CORE_MANAGEDOBJECT_HPP
