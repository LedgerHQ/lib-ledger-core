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

#include "logger.hpp"
#include <spdlog/spdlog.h>
#include "LogPrinterSink.hpp"
#include "RotatingEncryptableSink.hpp"

namespace ledger {
    namespace core {

        std::shared_ptr<spdlog::logger> logger::create(const std::string &name,
                                 std::experimental::optional<std::string> password,
                                 const std::shared_ptr<api::ExecutionContext> &context,
                                 const std::shared_ptr<api::PathResolver> &resolver,
                                 const std::shared_ptr<api::LogPrinter> &printer,
                                 size_t maxSize) {
            auto logPrinterSink = std::make_shared<LogPrinterSink>(printer);
            auto rotatingSink = std::make_shared<RotatingEncryptableSink>(context, resolver, name, password, maxSize, 3);
            auto logger = spdlog::create(name, {logPrinterSink, rotatingSink});
            spdlog::drop(name);
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);
            logger->set_pattern("%Y-%m-%dT%XZ%z %L: %v");
            return logger;
        }
    }
}