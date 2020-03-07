/*
 *
 * HorizonAccountParser.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 04/07/2019.
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

#include "HorizonAccountParser.hpp"
using namespace ledger::core;

static const JsonParserPathMatcher ACCOUNT_ID_MATCHER("/account_id");
static const JsonParserPathMatcher SEQUENCE_MATCHER("/sequence");
static const JsonParserPathMatcher FLAGS_MATCHER("/flags/?");
static const JsonParserPathMatcher BALANCES_MATCHER("/balances[*]/?");
static const JsonParserPathMatcher BALANCES_OBJECT_MATCHER("/balances[*]/");
static const JsonParserPathMatcher SUBENTRY_COUNT_MATCHER("/subentry_count");
static const JsonParserPathMatcher ACCOUNT_SIGNER_MATCHER("/signers[*]/");
static const JsonParserPathMatcher ACCOUNT_SIGNER_KEY_MATCHER("/signers[*]/key");
static const JsonParserPathMatcher ACCOUNT_SIGNER_TYPE_MATCHER("/signers[*]/type");
static const JsonParserPathMatcher ACCOUNT_SIGNER_WEIGHT_MATCHER("/signers[*]/weight");


namespace ledger {
    namespace core {

        bool HorizonAccountParser::Null() {
            return true;
        }

        bool HorizonAccountParser::Bool(bool b) {
            return true;
        }

        bool HorizonAccountParser::Int(int i) {
            return true;
        }

        bool HorizonAccountParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonAccountParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonAccountParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonAccountParser::Double(double d) {
            return true;
        }

        bool HorizonAccountParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(SUBENTRY_COUNT_MATCHER)) {
                _account->subentryCount = BigInt::fromString(std::string(str, length)).toInt();
            } else if (_path.match(ACCOUNT_SIGNER_WEIGHT_MATCHER) && !_account->signers.empty()) {
                _account->signers.back().weight = static_cast<int32_t>(BigInt::fromString(std::string(str, length)).toInt());
            }
            return true;
        }

        bool HorizonAccountParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(ACCOUNT_ID_MATCHER)) {
                _account->accountId = std::string(str, length);
            } else if (_path.match(SEQUENCE_MATCHER)) {
                _account->sequence = std::string(str, length);
            } else if (_path.match(BALANCES_MATCHER)) {
                _balancesParser.String(str, length, copy);
            } else if (_path.match(ACCOUNT_SIGNER_KEY_MATCHER) && !_account->signers.empty()) {
                _account->signers.back().key = std::string(str, length);
            } else if (_path.match(ACCOUNT_SIGNER_TYPE_MATCHER) && !_account->signers.empty()) {
                _account->signers.back().type = std::string(str, length);
            }
            return true;
        }

        bool HorizonAccountParser::StartObject() {
            if (_path.match(BALANCES_OBJECT_MATCHER)) {
                _account->balances.emplace_back(stellar::Balance());
                _balancesParser.init(&_account->balances.back());
            } else if (_path.match(ACCOUNT_SIGNER_MATCHER)) {
                _account->signers.emplace_back(stellar::AccountSigner());
            }
            return true;
        }

        bool HorizonAccountParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonAccountParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonAccountParser::StartArray() {
            return true;
        }

        bool HorizonAccountParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonAccountParser::init(stellar::Account *account) {
            _account = account;
        }

        void HorizonAccountParser::setPathView(const JsonParserPathView &path) {
            _path = path;
            _flagsParser.setPathView(_path.view(2));
            _balancesParser.setPathView(_path.view(4));
        }
    }
}