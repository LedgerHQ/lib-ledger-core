/*
 *
 * LockedResource
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/11/2016.
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
#ifndef LEDGER_CORE_LOCKEDRESOURCE_HPP
#define LEDGER_CORE_LOCKEDRESOURCE_HPP

#include "../api/Lock.hpp"

#include <memory>

namespace ledger {
    namespace core {
        template <typename T>
        class LockedResource {
          public:
            LockedResource(std::shared_ptr<api::Lock> lock, T resource) : _lock(lock), _resource(resource) {
                _isLocked = false;
            }

            T *operator->() {
                lock();
                return &_resource;
            }

            void lock() {
                if (!_isLocked) {
                    _isLocked = true;
                    _lock->lock();
                }
            }

            void swap(T value) {
                lock();
                _resource = value;
                unlock();
            }

            void unlock() {
                if (_isLocked) {
                    _isLocked = false;
                    _lock->unlock();
                }
            }

            ~LockedResource() {
                unlock();
            }

          private:
            std::shared_ptr<api::Lock> _lock;
            T _resource;
            bool _isLocked;
        };
    } // namespace core
} // namespace ledger

#endif // LEDGER_CORE_LOCKEDRESOURCE_HPP
