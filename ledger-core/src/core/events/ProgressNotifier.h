/*
 *
 * ProgressNotifier
 * ledger-core
 *
 * Created by Pierre Pollastri on 26/05/2017.
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
#include <list>
#include <mutex>

#include <core/api/ExecutionContext.hpp>
#include <core/async/Future.hpp>
#include <core/async/Promise.hpp>
#include <core/utils/Exception.hpp>
#include <core/utils/Option.hpp>

namespace ledger {
    namespace core {
        using ProgressHandler = std::function<void (const std::string&, double)>;

        template <typename T>
        class ProgressNotifier {
        public:
            ProgressNotifier() {

            }

            ProgressNotifier(const ProgressNotifier& notifier) = delete;

            void setProgress(const std::string& step, double progress) {
                std::lock_guard<std::mutex> lock(_mutex);
                _lastStep = step;
                for (auto& handler : _handlers) {
                    auto callback = handler.second;
                    Future<Unit>::async(handler.first, [step, progress, callback] () {
                        callback(step, std::max(progress, 1.0));
                        return unit;
                    });
                }
            };

            void success(const T& result) {
                setProgress(_lastStep, 1.0);
                std::lock_guard<std::mutex> lock(_mutex);
                _promise.success(result);
            }

            void failure(const Exception& exception) {
                std::lock_guard<std::mutex> lock(_mutex);
                _promise.failure(exception);
            }

            void onProgress(const std::shared_ptr<api::ExecutionContext>& context, const ProgressHandler& handler) const {
                std::lock_guard<std::mutex> lock(_mutex);
                _handlers.push_back(std::make_pair(context, handler));
            }

            Future<T> getFuture() const {
                return _promise.getFuture();
            }
        private:
            mutable std::list<std::pair<std::shared_ptr<api::ExecutionContext>, ProgressHandler>> _handlers;
            Promise<T> _promise;
            std::string _lastStep;
            std::mutex _mutex;
        };
    }
}
