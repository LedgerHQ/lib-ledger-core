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

#include "HorizonAssetParser.hpp"

using namespace ledger::core;

static const JsonParserPathMatcher TYPE_MATCHER("/asset_type");
static const JsonParserPathMatcher ISSUER_MATCHER("/asset_issuer");
static const JsonParserPathMatcher CODE_MATCHER("/asset_code");
static const JsonParserPathMatcher FLAGS_MATCHER("/flags/?");

namespace ledger {
    namespace core {

        bool HorizonAssetParser::Null() {
            return true;
        }

        bool HorizonAssetParser::Bool(bool b) {
            if (_path.match(FLAGS_MATCHER)) {
                _flagsParser.Bool(b);
            }
            return true;
        }

        bool HorizonAssetParser::Int(int i) {
            return true;
        }

        bool HorizonAssetParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonAssetParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonAssetParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonAssetParser::Double(double d) {
            return true;
        }

        bool HorizonAssetParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAssetParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            auto t = _path.toString();
            if (_path.match(TYPE_MATCHER)) {
                _asset->type = std::string(str, length);
            } else if (_path.match(ISSUER_MATCHER)) {
                _asset->issuer = std::string(str, length);
            } else if (_path.match(CODE_MATCHER)) {
                _asset->code = std::string(str, length);
            }
            return true;
        }

        bool HorizonAssetParser::StartObject() {
            return true;
        }

        bool HorizonAssetParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAssetParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonAssetParser::StartArray() {
            return true;
        }

        bool HorizonAssetParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonAssetParser::init(stellar::Asset *asset) {
            _asset = asset;
            _flagsParser.init(&_asset->flags);
        }

        void HorizonAssetParser::setPathView(const JsonParserPathView &view) {
            _path = view;
            _flagsParser.setPathView(_path.view(2));
        }

    }
}
