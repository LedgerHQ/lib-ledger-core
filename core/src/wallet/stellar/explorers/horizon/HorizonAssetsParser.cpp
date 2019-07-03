/*
 *
 * HorizonAssetsParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/06/2019.
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

#include "HorizonAssetsParser.hpp"

using namespace ledger::core;

const static JsonParserPathMatcher ASSET_MATCHER("/records[*]/?");
const static JsonParserPathMatcher ASSET_ARRAY_MATCHER("/records[*]/");

#define DELEGATE(x, ...) \
    if (_path.match(ASSET_MATCHER)) { \
    _assetParser.x(__VA_ARGS__); \
    } \
    return true; \

namespace ledger {
    namespace core {

        bool HorizonAssetsParser::Null() {
            DELEGATE(Null)
        }

        bool HorizonAssetsParser::Bool(bool b) {
            DELEGATE(Bool, b)
        }

        bool HorizonAssetsParser::Int(int i) {
            DELEGATE(Int, i)
        }

        bool HorizonAssetsParser::Uint(unsigned i) {
            DELEGATE(Uint, i)
        }

        bool HorizonAssetsParser::Int64(int64_t i) {
            DELEGATE(Int64, i)
        }

        bool HorizonAssetsParser::Uint64(uint64_t i) {
            DELEGATE(Uint64, i)
        }

        bool HorizonAssetsParser::Double(double d) {
            DELEGATE(Double, d)
        }

        bool HorizonAssetsParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            DELEGATE(RawNumber, str, length, copy)
        }

        bool HorizonAssetsParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            DELEGATE(String, str, length, copy)
        }

        bool HorizonAssetsParser::StartObject() {
            if (_path.match(ASSET_ARRAY_MATCHER)) {
                _assets->emplace_back(std::make_shared<stellar::Asset>());
                _assetParser.init(_assets->back().get());
            }
            DELEGATE(StartObject)
        }

        bool HorizonAssetsParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            DELEGATE(Key, str, length, copy)
        }

        bool HorizonAssetsParser::EndObject(rapidjson::SizeType memberCount) {
            DELEGATE(EndObject, memberCount)
        }

        bool HorizonAssetsParser::StartArray() {
            DELEGATE(StartArray)
        }

        bool HorizonAssetsParser::EndArray(rapidjson::SizeType elementCount) {
            DELEGATE(EndArray, elementCount)
        }

        void HorizonAssetsParser::init(std::vector<std::shared_ptr<stellar::Asset>> *assets) {
            _assets = assets;
        }

        void HorizonAssetsParser::setPathView(const JsonParserPathView &path) {
            _path = path;
            _assetParser.setPathView(_path.view(4));
        }
    }
}