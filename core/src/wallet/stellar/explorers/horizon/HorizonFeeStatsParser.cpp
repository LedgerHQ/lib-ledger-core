/*
 *
 * HorizonFeeStatsParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 09/07/2019.
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

#include "HorizonFeeStatsParser.hpp"

using namespace ledger::core;

static const JsonParserPathMatcher LAST_LEDGER_MATCHER("/last_ledger");
static const JsonParserPathMatcher BASE_FEE_MATCHER("/last_ledger_base_fee");
static const JsonParserPathMatcher MIN_FEE_MATCHER("/min_accepted_fee");
static const JsonParserPathMatcher MODE_FEE_MATCHER("/mode_accepted_fee");
static const JsonParserPathMatcher MAX_FEE_MATCHER("/p99_accepted_fee");

namespace ledger {
    namespace core {

        bool HorizonFeeStatsParser::Null() {
            return true;
        }

        bool HorizonFeeStatsParser::Bool(bool b) {
            return true;
        }

        bool HorizonFeeStatsParser::Int(int i) {
            return true;
        }

        bool HorizonFeeStatsParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonFeeStatsParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonFeeStatsParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonFeeStatsParser::Double(double d) {
            return true;
        }

        bool HorizonFeeStatsParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonFeeStatsParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(LAST_LEDGER_MATCHER)) {
                _feeStats->lastLedger = std::string(str, length);
            } else if (_path.match(MIN_FEE_MATCHER)) {
                _feeStats->minAccepted = BigInt::fromString(std::string(str, length));
            } else if (_path.match(MODE_FEE_MATCHER)) {
                _feeStats->modeAcceptedFee = BigInt::fromString(std::string(str, length));
            } else if (_path.match(MAX_FEE_MATCHER)) {
                _feeStats->maxFee = BigInt::fromString(std::string(str, length));
            } else if (_path.match(BASE_FEE_MATCHER)) {
                _feeStats->lastBaseFee = BigInt::fromString(std::string(str, length));
            }
            return true;
        }

        bool HorizonFeeStatsParser::StartObject() {
            return true;
        }

        bool HorizonFeeStatsParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonFeeStatsParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonFeeStatsParser::StartArray() {
            return true;
        }

        bool HorizonFeeStatsParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonFeeStatsParser::init(stellar::FeeStats *feeStats) {
            _feeStats = feeStats;
        }

        void HorizonFeeStatsParser::setPathView(const JsonParserPathView &path) {
            _path = path;
        }
    }
}