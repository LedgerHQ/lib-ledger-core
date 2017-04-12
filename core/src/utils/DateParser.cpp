/*
 *
 * DateParser
 * ledger-core
 *
 * Created by Pierre Pollastri on 11/04/2017.
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
#include "DateParser.hpp"
#include <regex>
#include "Exception.hpp"
#include <boost/lexical_cast.hpp>
#include <ctime>

std::chrono::system_clock::time_point ledger::core::DateParser::fromJSON(const std::string &str) {
    std::regex ex("([0-9]+)-([0-9]+)-([0-9]+)T([0-9]+):([0-9]+):([0-9]+)[\\.0-9]*([Z]?)");
    std::cmatch what;
    if (regex_match(str.c_str(), what, ex)) {
        auto year = boost::lexical_cast<unsigned int>(std::string(what[1].first, what[1].second));
        auto month = boost::lexical_cast<unsigned int>(std::string(what[2].first, what[2].second));
        auto day = boost::lexical_cast<unsigned int>(std::string(what[3].first, what[3].second));
        auto hours = boost::lexical_cast<unsigned int>(std::string(what[4].first, what[4].second));
        auto minutes = boost::lexical_cast<unsigned int>(std::string(what[5].first, what[5].second));
        auto seconds = boost::lexical_cast<unsigned int>(std::string(what[6].first, what[6].second));
        auto localTime = std::string(what[7].first, what[7].second);

        if (localTime != "Z") {
            throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT, "Cannot create date from local date {}", str);
        }

        struct tm time;
        time.tm_year = year - 1900;
        time.tm_mon = month - 1;
        time.tm_hour = hours;
        time.tm_mday = day;
        time.tm_hour = hours;
        time.tm_min = minutes;
        time.tm_sec = seconds;
        return std::chrono::system_clock::from_time_t(timegm(&time));
    } else {
        throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT, "Cannot convert {} to date", str);
    }
}
