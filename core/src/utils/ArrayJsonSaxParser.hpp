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

#ifndef ARRAY_JSON_SAX_PARSER_HPP
#define ARRAY_JSON_SAX_PARSER_HPP

#include <rapidjson/reader.h>

namespace ledger {
    namespace core {

        /**
         * Helper class to parse a JSON array with rapidjson SAX parser nad LedgerApiParser like classes. Note that
         * this parser only works with arrays of JSON objects.
         */
        template <class Parser>
        class ArrayJsonSaxParser {
        public:
            typedef std::vector<std::shared_ptr<typename Parser::Result>> Result;
            ArrayJsonSaxParser(std::string& lastKey) : _lastKey(lastKey), _parser(lastKey), _current(nullptr), _depth(0) {};

            bool Null() {
                return _parser.Null();
            }

            bool Bool(bool b) {
                return _parser.Bool(b);
            }

            bool Int(int i) {
                return _parser.Int(i);
            }

            bool Uint(unsigned i) {
                return _parser.Uint(i);
            }

            bool Int64(int64_t i) {
                return _parser.Int64(i);
            }

            bool Uint64(uint64_t i) {
                return _parser.Uint64(i);
            }

            bool Double(double d) {
                return _parser.Double(d);
            }

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                return _parser.RawNumber(str, length, copy);
            }

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                return _parser.String(str, length, copy);
            }

            bool StartObject() {
                _depth += 1;
                if (_current != nullptr)
                    return _parser.StartObject();
                if (_depth == 1) {
                    auto obj = std::make_shared<typename Parser::Result>();
                    _result->emplace_back(obj);
                    _current = & _result->back().get();
                }
                return true;
            }

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                return _parser.Key(str, length, copy);
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                _depth -= 1;
                if (_current != nullptr)
                    return _parser.EndObject(memberCount);
                if (_depth == 0) {
                    _current = nullptr;
                }
                return true;
            }

            bool StartArray() {
                return _parser.StartArray();
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                return _parser.EndArray(elementCount);
            }

            void init(std::vector<typename Parser::Result> *result) {
                _result = result;
            }

        private:
            int _depth;
            Parser _parser;
            std::string& _lastKey;
            std::vector<std::shared_ptr<typename Parser::Result>>* _result;
            typename Parser::Result* _current;
        };

    }
}

#endif
