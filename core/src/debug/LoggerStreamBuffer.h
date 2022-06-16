/*
 *
 * LoggerStreamBuffer
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/07/2017.
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
#ifndef LEDGER_CORE_LOGGERSTREAMBUFFER_H
#define LEDGER_CORE_LOGGERSTREAMBUFFER_H

#include "logger.hpp"

#include <sstream>

namespace ledger {
    namespace core {
        class LoggerStreamBuffer : public std::streambuf {
          public:
            LoggerStreamBuffer(const std::string &tag, const std::shared_ptr<spdlog::logger> &logger);

          protected:
            int overflow(int c) override;

          private:
            std::stringstream _buffer;
            std::shared_ptr<spdlog::logger> _logger;
            std::string _tag;
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_LOGGERSTREAMBUFFER_H
