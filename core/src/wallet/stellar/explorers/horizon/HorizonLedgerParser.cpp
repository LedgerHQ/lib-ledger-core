/*
 *
 * HorizonLedgerParser.cpp
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

#include "HorizonLedgerParser.hpp"
#include <utils/DateUtils.hpp>

using namespace ledger::core;

static const JsonParserPathMatcher HASH_MATCHER("/hash");
static const JsonParserPathMatcher SEQUENCE_MATCHER("/sequence");
static const JsonParserPathMatcher TIME_MATCHER("/closed_at");
static const JsonParserPathMatcher FEE_MATCHER("/base_fee_in_stroops");
static const JsonParserPathMatcher RESERVE_MATCHER("/base_reserve_in_stroops");

namespace ledger {
    namespace core {

        bool HorizonLedgerParser::Null() {
            return true;
        }

        bool HorizonLedgerParser::Bool(bool b) {
            return true;
        }

        bool HorizonLedgerParser::Int(int i) {
            return true;
        }

        bool HorizonLedgerParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonLedgerParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonLedgerParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonLedgerParser::Double(double d) {
            return true;
        }

        bool HorizonLedgerParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(SEQUENCE_MATCHER)) {
                _ledger->height = BigInt::fromString(std::string(str, length)).toUint64();
            } else if (_path.match(FEE_MATCHER)) {
                _ledger->baseFee = BigInt::fromString(std::string(str, length));
            } else if (_path.match(RESERVE_MATCHER)) {
                _ledger->baseReserve = BigInt::fromString(std::string(str, length));
            }
            return true;
        }

        bool HorizonLedgerParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(HASH_MATCHER)) {
                _ledger->hash = std::string(str, length);
            } else if (_path.match(TIME_MATCHER)) {
                _ledger->time = DateUtils::fromJSON(std::string(str, length));
            }
            return true;
        }

        bool HorizonLedgerParser::StartObject() {
            return true;
        }

        bool HorizonLedgerParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonLedgerParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonLedgerParser::StartArray() {
            return true;
        }

        bool HorizonLedgerParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonLedgerParser::init(stellar::Ledger *ledger) {
            _ledger = ledger;
        }

        void HorizonLedgerParser::setPathView(const JsonParserPathView &path) {
            _path = path;
        }
    }
}