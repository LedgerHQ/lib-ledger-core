/*
 *
 * HorizonBalanceParser.cpp
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

#include "HorizonBalanceParser.hpp"

using namespace ledger::core;

static const JsonParserPathMatcher BALANCE_MATCHER("/balance");
static const JsonParserPathMatcher BUYING_LIABILITIES_MATCHER("/buying_liabilities");
static const JsonParserPathMatcher SELLING_LIABILITIES_MATCHER("/selling_liabilities");
static const JsonParserPathMatcher ASSET_TYPE_MATCHER("/asset_type");
static const JsonParserPathMatcher ASSET_ISSUER_MATCHER("/asset_issuer");
static const JsonParserPathMatcher ASSET_CODE_MATCHER("/asset_code");

namespace ledger {
    namespace core {

        bool HorizonBalanceParser::Null() {
            return true;
        }

        bool HorizonBalanceParser::Bool(bool b) {
            return true;
        }

        bool HorizonBalanceParser::Int(int i) {
            return true;
        }

        bool HorizonBalanceParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonBalanceParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonBalanceParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonBalanceParser::Double(double d) {
            return true;
        }

        bool HorizonBalanceParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonBalanceParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            auto t = _path.toString();
            if (_path.match(BALANCE_MATCHER)) {
                _balance->value = BigInt::fromFloatString(std::string(str, length), 7);
            } else if (_path.match(SELLING_LIABILITIES_MATCHER)) {
                _balance->sellingLiabilities = Option<BigInt>(BigInt::fromFloatString(std::string(str, length), 7));
            } else if (_path.match(BUYING_LIABILITIES_MATCHER)) {
                _balance->buyingLiabilities = Option<BigInt>(BigInt::fromFloatString(std::string(str, length), 7));
            } else if (_path.match(ASSET_TYPE_MATCHER)) {
                _balance->assetType = std::string(str, length);
            } else if (_path.match(ASSET_CODE_MATCHER)) {
                _balance->assetCode = Option<std::string>(std::string(str, length));
            } else if (_path.match(ASSET_ISSUER_MATCHER)) {
                _balance->assetIssuer = Option<std::string>(std::string(str, length));
            }
            return true;
        }

        bool HorizonBalanceParser::StartObject() {
            return true;
        }

        bool HorizonBalanceParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonBalanceParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonBalanceParser::StartArray() {
            return true;
        }

        bool HorizonBalanceParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonBalanceParser::init(stellar::Balance *balance) {
            _balance = balance;
        }

        void HorizonBalanceParser::setPathView(const JsonParserPathView &path) {
            _path = path;
        }
    }
}