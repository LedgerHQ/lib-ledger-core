/*
 *
 * ledger-core
 *
 * Created by Pierre Pollastri.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef HORIZON_API_PARSER_HPP
#define HORIZON_API_PARSER_HPP

#include <rapidjson/reader.h>
#include <utils/JsonParserPath.hpp>

namespace ledger {
    namespace core {
        struct HorizonError {
            uint32_t statusCode;
            std::string statusMessage;
            std::string errorTitle;
            std::string errorDetails;

            HorizonError() : statusCode(0) {};
        };

        template <class ResultType, class Parser, bool HasEmbedded = true>
        class HorizonApiParser {
        public:
            using Result = ResultType;
            using Response = Either<Exception, std::shared_ptr<ResultType>>;

            HorizonApiParser() : _contentMatcher("/_embedded") {
                _result = std::make_shared<ResultType>();
                _parser.init(_result.get());
                _parser.setPathView(_path.view(HasEmbedded ? 2 : 0));
            }

            HorizonApiParser(const HorizonApiParser<ResultType, Parser, HasEmbedded>& cpy) :
                _error(cpy._error),
                _result(cpy._result),
                _path(cpy._path),
                _contentMatcher("/_embedded") {
                _parser.init(_result.get());
                _parser.setPathView(_path.view(HasEmbedded ? 2 : 0));
            }

            bool Null() {
                _path.value();
                return delegate([&] () {
                   _parser.Null();
                });
            }

            bool Bool(bool b) {
                _path.value();
                return delegate([&] () {
                    _parser.Bool(b);
                });
            }

            bool Int(int i) {
                _path.value();
                return delegate([&] () {
                    _parser.Int(i);
                });
            }

            bool Uint(unsigned i) {
                _path.value();
                return delegate([&] () {
                   _parser.Uint(i);
                });
            }

            bool Int64(int64_t i) {
                _path.value();
                return delegate([&] () {
                   _parser.Int64(i);
                });
            }

            bool Uint64(uint64_t i) {
                _path.value();
                return delegate([&] () {
                   _parser.Uint64(i);
                });
            }

            bool Double(double d) {
                _path.value();
                return delegate([&] () {
                   _parser.Double(d);
                });
            }

            bool RawNumber(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                _path.value();
                return delegate([&] () {
                    _parser.RawNumber(str, length, copy);
                });
            }

            bool String(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                static JsonParserPathMatcher titlePath("/title");
                static JsonParserPathMatcher detailsPath("/details");
                _path.value();

                if (_path.view(0).match(titlePath)) {
                    _error.errorTitle = std::string(str, length);
                } else if (_path.view(0).match(detailsPath)) {
                    _error.errorDetails = std::string(str, length);
                }
                return delegate([&] () {
                    _parser.String(str, length, copy);
                });
            }

            bool StartObject() {
                _path.startObject();
                return delegate([&] () {
                    _parser.StartObject();
                });
            }

            bool Key(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                _path.key(std::string(str, length));
                delegate([&] () {
                    _parser.Key(str, length, copy);
                });
                return continueParsing();
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                _path.endObject();
                delegate([&] () {
                    _parser.EndObject(memberCount);
                });
                return continueParsing();
            }

            bool StartArray() {
                _path.startArray();
                delegate([&] () {
                    _parser.StartArray();
                });
                return continueParsing();
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                _path.endArray();
                delegate([&] () {
                    _parser.EndArray(elementCount);
                });
                return continueParsing();
            }

            Either<Exception, std::shared_ptr<ResultType>> build() {
                if (isFailure()) {
                    auto error = std::make_shared<HorizonError>(_error);
                    auto ex = make_exception(api::ErrorCode::API_ERROR, std::static_pointer_cast<void>(error), "{} - {}: {}", _error.statusCode, _error.errorTitle, _error.errorDetails);
                    return Either<Exception, std::shared_ptr<ResultType>>(ex);
                } else {
                    return Either<Exception, std::shared_ptr<ResultType>>(_result);
                }
            };

            void attach(const std::shared_ptr<api::HttpUrlConnection>& connection) {
                _error.statusCode = (uint32_t) connection->getStatusCode();
                _error.statusMessage = connection->getStatusText();
            }

            void attach(const std::string& statusText, uint32_t statusCode) {
                _error.statusCode = statusCode;
                _error.statusMessage = statusText;
            }

            inline bool isFailure() const {
                return _error.statusCode < 200 || _error.statusCode >= 400;
            }

            inline bool continueParsing() const {
                return _exception.isEmpty();
            }

        private:
            bool delegate(std::function<void ()>&& fn) {
                if ((!HasEmbedded && !isFailure()) || (!isFailure() && _path.match(_contentMatcher, 0))) {
                    _exception = Try<Unit>::from([&] () {
                        fn();
                        return unit;
                    }).exception();
                }
                return continueParsing();
            }
        private:
            Parser _parser;
            JsonParserPathMatcher _contentMatcher;
            std::shared_ptr<ResultType> _result;
            HorizonError _error;
            JsonParserPath _path;
            Option<Exception> _exception;
        };
    }
}

#endif
