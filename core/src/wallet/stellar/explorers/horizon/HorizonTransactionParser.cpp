/*
 *
 * HorizonTransactionParser.cpp
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

#include "HorizonTransactionParser.hpp"
#include <utils/DateUtils.hpp>
#include <wallet/stellar/xdr/XDRDecoder.hpp>
#include <math/BaseConverter.hpp>

using namespace ledger::core;

static const JsonParserPathMatcher HASH_MATCHER("/hash");
static const JsonParserPathMatcher SUCCESSFUL_MATCHER("/successful");
static const JsonParserPathMatcher LEDGER_MATCHER("/ledger");
static const JsonParserPathMatcher DATE_MATCHER("/created_at");
static const JsonParserPathMatcher ACCOUNT_MATCHER("/source_account");
static const JsonParserPathMatcher ACCOUNT_SEQUENCE_MATCHER("/source_account_sequence");
static const JsonParserPathMatcher FEE_MATCHER("/fee_charged");
static const JsonParserPathMatcher MEMO_TYPE_MATCHER("/memo_type");
static const JsonParserPathMatcher MEMO_MATCHER("/memo");
static const JsonParserPathMatcher PAGING_TOKEN_MATCHER("/paging_token");
static const JsonParserPathMatcher ENVELOPE_XDR("/envelope_xdr");

namespace ledger {
    namespace core {

        bool HorizonTransactionParser::Null() {
            return true;
        }

        bool HorizonTransactionParser::Bool(bool b) {
            if (_path.match(SUCCESSFUL_MATCHER)) {
                _transaction->successful = b;
            }
            return true;
        }

        bool HorizonTransactionParser::Int(int i) {
            return true;
        }

        bool HorizonTransactionParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonTransactionParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonTransactionParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonTransactionParser::Double(double d) {
            return true;
        }

        bool
        HorizonTransactionParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
           if (_path.match(LEDGER_MATCHER)) {
                _transaction->ledger = BigInt::fromString(std::string(str, length)).toUint64();
            }
            return true;
        }

        bool HorizonTransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(HASH_MATCHER)) {
                _transaction->hash = std::string(str, length);
            } else if (_path.match(ACCOUNT_MATCHER)) {
                _transaction->sourceAccount = std::string(str, length);
            } else if (_path.match(DATE_MATCHER)) {
                _transaction->createdAt = DateUtils::fromJSON(std::string(str, length));
            } else if (_path.match(MEMO_MATCHER)) {
                _transaction->memo = std::string(str, length);
            } else if (_path.match(MEMO_TYPE_MATCHER)) {
                _transaction->memoType = std::string(str, length);
            } else if (_path.match(ACCOUNT_SEQUENCE_MATCHER)) {
                _transaction->sourceAccountSequence = BigInt::fromString(std::string(str, length));
            } else if (_path.match(PAGING_TOKEN_MATCHER)) {
                _transaction->pagingToken = std::string(str, length);
            } else if (_path.match(ENVELOPE_XDR)) {
                std::vector<uint8_t> buffer;
                BaseConverter::decode(std::string(str, length), BaseConverter::BASE64_RFC4648, buffer);
                stellar::xdr::Decoder decoder(buffer);
                decoder >> _transaction->envelope;
            } else if (_path.match(FEE_MATCHER)) {
                _transaction->feePaid = BigInt::fromString(std::string(str, length));
            } else if (_path.match(ACCOUNT_SEQUENCE_MATCHER)) {
                _transaction->sourceAccountSequence = BigInt::fromString(std::string(str, length));
            }
            return true;
        }

        bool HorizonTransactionParser::StartObject() {
            return true;
        }

        bool HorizonTransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonTransactionParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonTransactionParser::StartArray() {
            return true;
        }

        bool HorizonTransactionParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonTransactionParser::init(stellar::Transaction *transaction) {
            _transaction = transaction;
        }

        void HorizonTransactionParser::setPathView(const JsonParserPathView &path) {
            _path = path;
        }
    }
}