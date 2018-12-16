/*
 *
 * LedgerApiParser
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/04/2017.
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
#ifndef LEDGER_CORE_LEDGERAPIPARSER_HPP
#define LEDGER_CORE_LEDGERAPIPARSER_HPP

#include <cstdint>
#include <rapidjson/reader.h>
#include <net/HttpClient.hpp>
#include <collections/collections.hpp>

namespace ledger {
    namespace core {
        template <typename ResultType, typename Parser>
        class LedgerApiParser {
        public:
            LedgerApiParser() : _parser(_lastKey) {
                _statusCode = 0;
                _depth = 0;
                _result = std::make_shared<ResultType>();
                _parser.init(_result.get());
            }

            LedgerApiParser(const LedgerApiParser<ResultType, Parser>& cpy) :
                _statusCode(cpy._statusCode),
                _statusText(cpy._statusText),
                _depth(cpy._depth),
                _result(cpy._result),
                _lastKey(cpy._lastKey),
                _parser(_lastKey) {
                _parser.init(_result.get());
            }

            bool Null() {
               return delegate([&] () {
                   _parser.Null();
               });
            }

            bool Bool(bool b) {
               return delegate([&] () {
                    _parser.Bool(b);
               });
            }

            bool Int(int i) {
                return delegate([&] () {
                    _parser.Int(i);
                });
            }

            bool Uint(unsigned i) {
                return delegate([&] () {
                   _parser.Uint(i);
                });
            }

            bool Int64(int64_t i) {
                return delegate([&] () {
                   _parser.Int64(i);
                });
            }

            bool Uint64(uint64_t i) {
                return delegate([&] () {
                   _parser.Uint64(i);
                });
            }

            bool Double(double d) {
                return delegate([&] () {
                   _parser.Double(d);
                });
            }

            bool RawNumber(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                return delegate([&] () {
                    _parser.RawNumber(str, length, copy);
                });
            }

            bool String(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                if (_depth == 1 && isFailure() && _lastKey == "error") {
                    _error = std::string(str, length);
                }
                return delegate([&] () {
                    _parser.String(str, length, copy);
                });
            }

            bool StartObject() {
                _depth += 1;
                return delegate([&] () {
                    _parser.StartObject();
                });
            }

            bool Key(const rapidjson::Reader::Ch* str, rapidjson::SizeType length, bool copy) {
                _lastKey = std::string(str, length);
                delegate([&] () {
                    _parser.Key(str, length, copy);
                });
                return continueParsing();
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                _depth -= 1;
                delegate([&] () {
                    _parser.EndObject(memberCount);
                });
                return continueParsing();
            }

            bool StartArray() {
                _depth += 1;
                delegate([&] () {
                    _parser.StartArray();
                });
                return continueParsing();
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                _depth -= 1;
                delegate([&] () {
                    _parser.EndArray(elementCount);
                });
                return continueParsing();
            }

            Either<Exception, std::shared_ptr<ResultType>> build() {
                if (isFailure()) {
                    auto ex = make_exception(api::ErrorCode::API_ERROR, "{} - {}: {}", _statusCode, _statusText, _error);
                    return Either<Exception, std::shared_ptr<ResultType>>(ex);
                } else {
                    return Either<Exception, std::shared_ptr<ResultType>>(_result);
                }
            };

            void attach(const std::shared_ptr<api::HttpUrlConnection>& connection) {
                _statusCode = (uint32_t) connection->getStatusCode();
                _statusText = connection->getStatusText();
            }

            void attach(const std::string& statusText, uint32_t statusCode) {
                _statusCode = statusCode;
                _statusText = _statusText;
            }

            inline bool isFailure() const {
                return _statusCode < 200 || _statusCode >= 400;
            }

            inline bool continueParsing() const {
                return _exception.isEmpty();
            }

        private:
            bool delegate(std::function<void ()>&& fn) {
                if (!isFailure()) {
                    _exception = Try<Unit>::from([&] () {
                        fn();
                        return unit;
                    }).exception();
                }
                return continueParsing();
            }
        private:
            Parser _parser;
            std::shared_ptr<ResultType> _result;
            uint32_t _depth;
            std::string _statusText;
            uint32_t _statusCode;
            std::string _error;
            std::string _lastKey;
            Option<Exception> _exception;
        };
    }
}

#endif //LEDGER_CORE_LEDGERAPIPARSER_HPP
