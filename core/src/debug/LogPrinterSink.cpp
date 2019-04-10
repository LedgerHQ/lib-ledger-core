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
#include "LogPrinterSink.hpp"
#include "utils/LambdaRunnable.hpp"
#include "api/LogPrinter.hpp"
#include "api/ExecutionContext.hpp"

namespace ledger {
    namespace core {

        LogPrinterSink::LogPrinterSink(const std::shared_ptr<api::LogPrinter> &printer) {
            _printer = printer;
            set_level(spdlog::level::trace);
        }

        void LogPrinterSink::sink_it_(const spdlog::details::log_msg &msg) {
            auto printer = _printer.lock();
            if (!printer)
                return;
            auto level = msg.level;
            
            fmt::memory_buffer buffer;
            formatter_->format(msg, buffer);
            std::string message(buffer.data(), buffer.size());
            printer->getContext()->execute(make_runnable([printer, level, message]() {
                switch (level) {
                    case spd::level::trace:
                        printer->printApdu(message);
                        break;
                    case spdlog::level::debug:
                        printer->printDebug(message);
                        break;
                    case spdlog::level::info:
                        printer->printInfo(message);
                        break;
                    case spdlog::level::warn:
                        printer->printWarning(message);
                        break;
                    case spdlog::level::err:
                        printer->printError(message);
                        break;
                    case spdlog::level::critical:
                        printer->printCriticalError(message);
                        break;
                    case spdlog::level::off:
                        break;
                }
            }));
        }

        void LogPrinterSink::flush_() {

        }
    }
}