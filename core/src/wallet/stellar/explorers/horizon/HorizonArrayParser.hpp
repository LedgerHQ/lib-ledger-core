/*
 *
 * HorizonArrayParser.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/07/2019.
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

#ifndef LEDGER_CORE_HORIZONARRAYPARSER_HPP
#define LEDGER_CORE_HORIZONARRAYPARSER_HPP

#include <rapidjson/reader.h>
#include <utils/JsonParserPath.hpp>
#include <fmt/printf.h>

namespace ledger {
    namespace core {

    #define DELEGATE(x, ...) \
        if (_path.match(_itemMatcher)) { \
            _parser.x(__VA_ARGS__); \
        } \
        return true;

        template <class Parser, class Item>
        class HorizonArrayParser {
        public:

            HorizonArrayParser() : _itemMatcher("/records[*]/?"),
                                   _arrayMatcher("/records[*]/")

            {

            }

            bool Null() {
                DELEGATE(Null)
            }

            bool Bool(bool b) {
                DELEGATE(Bool, b)
            }

            bool Int(int i) {
                DELEGATE(Int, i)
            }

            bool Uint(unsigned i) {
                DELEGATE(Uint, i)
            }

            bool Int64(int64_t i) {
                DELEGATE(Int64, i)
            }

            bool Uint64(uint64_t i) {
                DELEGATE(Uint64, i)
            }

            bool Double(double d) {
                DELEGATE(Double, d)
            }

            bool RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                DELEGATE(RawNumber, str, length, copy)
            }

            bool String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                DELEGATE(String, str, length, copy)
            }

            bool StartObject() {
                if (_path.match(_arrayMatcher)) {
                    _array->emplace_back(std::make_shared<Item>());
                    _parser.init(_array->back().get());
                }
                DELEGATE(StartObject)
            }

            bool Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
                DELEGATE(Key, str, length, copy)
            }

            bool EndObject(rapidjson::SizeType memberCount) {
                DELEGATE(EndObject, memberCount)
            }

            bool StartArray() {
                DELEGATE(StartArray)
            }

            bool EndArray(rapidjson::SizeType elementCount) {
                DELEGATE(EndArray, elementCount)
            }

            void init(std::vector<std::shared_ptr<Item>> *array) {
                _array = array;
            }

            void setPathView(const JsonParserPathView& path) {
                _path = path;
                _parser.setPathView(_path.view(4));
            }

        private:
            JsonParserPathMatcher _arrayMatcher;
            JsonParserPathMatcher _itemMatcher;
            JsonParserPathView _path;
            Parser _parser;
            std::vector<std::shared_ptr<Item>>* _array;
        };

    #undef DELEGATE
    }
}

#endif //LEDGER_CORE_HORIZONARRAYPARSER_HPP
