/*
 *
 * URI
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/03/2017.
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
#include "URI.hpp"
#include <regex>
#include <boost/lexical_cast.hpp>

namespace ledger {
    namespace core {


        URI::URI(const String &uri) {
            std::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
            std::cmatch what;
            if(regex_match(uri.toCString(), what, ex))
            {
                _scheme =  std::string(what[1].first, what[1].second);
                _domain = std::string(what[2].first, what[2].second);
                _port = std::string(what[3].first, what[3].second);
                _path = std::string(what[4].first, what[4].second);
                _query = std::string(what[5].first, what[5].second);
                _fragment = std::string(what[6].first, what[6].second);
            }
        }

        const String &URI::getScheme() const {
            return _scheme;
        }

        const String &URI::getDomain() const {
            return _domain;
        }

        short URI::getPort() const {
            return boost::lexical_cast<short>(_port.str());
        }

        const String &URI::getPath() const {
            return _path;
        }

        const String &URI::getQuery() const {
            return _query;
        }

        const String &URI::getFragment() const {
            return _fragment;
        }

    }
}