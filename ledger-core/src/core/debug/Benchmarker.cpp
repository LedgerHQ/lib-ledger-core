/*
 *
 * Benchmarker
 * ledger-core
 *
 * Created by Pierre Pollastri on 01/08/2017.
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

#include <core/debug/Benchmarker.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/utils/DurationUtils.hpp>

namespace ledger {
    namespace core {
        Benchmarker::Benchmarker(const std::string &name, const std::shared_ptr<spdlog::logger> &logger) {
            _name = name;
            _logger = logger;
        }

        Benchmarker &Benchmarker::start() {
            _startDate = std::chrono::steady_clock::now();
            if (_logger) {
                _logger->info("{} started.", _name);
            }
            return *this;
        }

        Benchmarker &Benchmarker::stop() {
            _stopDate = std::chrono::steady_clock::now();
            if (_logger) {
                _logger->info("{} took {}.", _name, DurationUtils::formatDuration(getDuration()));
            } else {
                fmt::print("{} took {}.\n", _name, DurationUtils::formatDuration(getDuration()));
            }
            return *this;
        }

        std::chrono::steady_clock::duration Benchmarker::getDuration() const {
            return _stopDate - _startDate;
        }
    }
}
