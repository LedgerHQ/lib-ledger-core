/*
 *
 * logger
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/11/2016.
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
#ifndef LEDGER_CORE_LOGGER_HPP
#define LEDGER_CORE_LOGGER_HPP

#include <spdlog/spdlog.h>
#include "../api/ExecutionContext.hpp"
#include "../api/LogPrinter.hpp"
#include "../api/PathResolver.hpp"
#include <memory>
#include <cstddef>
#include "../utils/optional.hpp"

namespace ledger {
    namespace core {
        class logger {
        public:
            static const std::size_t DEFAULT_MAX_SIZE = 5 * 1048576;
            static std::shared_ptr<spdlog::logger> create(
                    const std::string& name,
                    std::experimental::optional<std::string> password,
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const std::shared_ptr<api::PathResolver>& resolver,
                    const std::shared_ptr<api::LogPrinter>& printer,
                    std::size_t maxSize = DEFAULT_MAX_SIZE,
                    bool disabled = false
            );
        private:
            logger() = delete;
        };
    }
}

#endif //LEDGER_CORE_LOGGER_HPP
