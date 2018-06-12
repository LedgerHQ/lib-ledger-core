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
#include "DateUtils.hpp"
#include <regex>
#include "Exception.hpp"
#include <boost/lexical_cast.hpp>
#include <ctime>

#ifdef __MINGW32__
time_t timegm(struct tm* tm) { return mktime(tm) - timezone; }
#endif

std::chrono::system_clock::time_point ledger::core::DateUtils::fromJSON(const std::string &str) {
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
        return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(timegm(&time)));
    } else {
        throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT, "Cannot convert {} to date", str);
    }
}

std::string ledger::core::DateUtils::toJSON(const std::chrono::system_clock::time_point &date) {
    std::tm tm = {0};
    std::time_t tt = std::chrono::system_clock::to_time_t(date);
#ifdef __MINGW32__
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    return fmt::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}Z", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

std::chrono::system_clock::time_point ledger::core::DateUtils::now() {
    return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
}

std::chrono::system_clock::time_point ledger::core::DateUtils::incrementDate(const std::chrono::system_clock::time_point &date,
                                                           api::TimePeriod precision) {
    std::tm tm = {0};
    std::time_t tt = std::chrono::system_clock::to_time_t(date);
#ifdef __MINGW32__
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif

    switch (precision){
        case api::TimePeriod::DAY:
            tm.tm_mday += 1;
            break;
        case api::TimePeriod::WEEK:
            tm.tm_mday += 7;
            break;
        case api::TimePeriod::MONTH:
            tm.tm_mon += 1;
            break;
        default:
            break;
    }
    return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(timegm(&tm)));
}
