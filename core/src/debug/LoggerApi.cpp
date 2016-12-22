/*
 *
 * LoggerApi
 * ledger-core
 *
 * Created by Pierre Pollastri on 22/12/2016.
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
#include "LoggerApi.hpp"

void ledger::core::LoggerApi::d(const std::string &tag, const std::string &message) {
    auto i = _logger.lock();
    if (i) i->debug("[{}] {}", tag, message);
}

void ledger::core::LoggerApi::i(const std::string &tag, const std::string &message) {
    auto i = _logger.lock();
    if (i) i->info("[{}] {}", tag, message);
}

void ledger::core::LoggerApi::e(const std::string &tag, const std::string &message) {
    auto i = _logger.lock();
    if (i) i->error("[{}] {}", tag, message);
}

void ledger::core::LoggerApi::w(const std::string &tag, const std::string &message) {
    auto i = _logger.lock();
    if (i) i->warn("[{}] {}", tag, message);
}

void ledger::core::LoggerApi::c(const std::string &tag, const std::string &message) {
    auto i = _logger.lock();
    if (i) i->critical("[{}] {}", tag, message);
}
