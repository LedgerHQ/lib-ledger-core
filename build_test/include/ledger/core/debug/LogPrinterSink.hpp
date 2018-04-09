/*
 *
 * LogPrinterSink
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
#ifndef LEDGER_CORE_LOGPRINTERSINK_HPP
#define LEDGER_CORE_LOGPRINTERSINK_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include "../api/LogPrinter.hpp"
#include <memory>

namespace spd = spdlog;

namespace ledger {
    namespace core {
        class LogPrinterSink : public spd::sinks::sink {
        public:
            LogPrinterSink(const std::shared_ptr<api::LogPrinter>& printer);

            virtual void log(const spdlog::details::log_msg &msg) override;

            virtual void flush() override;

        private:
            std::weak_ptr<api::LogPrinter> _printer;
        };
    }
}


#endif //LEDGER_CORE_LOGPRINTERSINK_HPP
