/*
 *
 * CompletionBlock
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/03/2017.
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
#ifndef LEDGER_CORE_COMPLETIONBLOCK_HPP
#define LEDGER_CORE_COMPLETIONBLOCK_HPP

#include "../traits/completion_block_traits.hpp"
#include "../async/Promise.hpp"
#include "../async/Future.hpp"

namespace ledger {
    namespace core {

        template <typename T, class Class, bool usesPtrInComplete>
        class CompletionBlock {

        };

        template <typename T, class Class>
        class CompletionBlock<T, Class, true> : public Class {
        public:
            virtual void complete(const std::shared_ptr<T>& result, const std::ledger_exp::optional<api::Error>& error) override {
                if (error) {
                    _promise.failure(Exception(error.value().code, error.value().message));
                } else {
                    _promise.success(result);
                }
            };

            Future<std::shared_ptr<T>> getFuture() const {
                return _promise.getFuture();
            }

        private:
            Promise<std::shared_ptr<T>> _promise;
        };

        template <typename T, class Class>
        class CompletionBlock<T, Class, false> : public Class {
        public:
            virtual void complete(const std::ledger_exp::optional<T>& result, const std::ledger_exp::optional<api::Error>& error) override {
                if (error) {
                    _promise.failure(Exception(error.value().code, error.value().message));
                } else {
                    _promise.success(result.value());
                }
            };

            Future<T> getFuture() const {
                return _promise.getFuture();
            }

        private:
            Promise<T> _promise;
        };

        template <typename T, class Class>
        std::shared_ptr<CompletionBlock<T, Class, has_complete_method<Class, void (const std::shared_ptr<T>&, const std::ledger_exp::optional<api::Error>&)>::value>>
        make_api_completion_block() {
            return std::make_shared<CompletionBlock<T, Class, has_complete_method<Class, void (const std::shared_ptr<T>&, const std::ledger_exp::optional<api::Error>&)>::value>>();
        };

    }
}


#endif //LEDGER_CORE_COMPLETIONBLOCK_HPP
