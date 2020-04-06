/*
 *
 * HorizonOperationParser.cpp
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

#include "HorizonOperationParser.hpp"
#include <utils/DateUtils.hpp>

using namespace ledger::core;

static const JsonParserPathMatcher ACCOUNT_MATCHER("/account");
static const JsonParserPathMatcher SUCCESSFUL_MATCHER("/transaction_successful");
static const JsonParserPathMatcher FUNDER_MATCHER("/funder");
static const JsonParserPathMatcher STARTING_BALANCE_MATCHER("/starting_balance");
static const JsonParserPathMatcher DATE_MATCHER("/created_at");
static const JsonParserPathMatcher TYPE_MATCHER("/type_i");
static const JsonParserPathMatcher TRANSACTION_HASH_MATCHER("/transaction_hash");
static const JsonParserPathMatcher FROM_MATCHER("/from");
static const JsonParserPathMatcher TO_MATCHER("/to");
static const JsonParserPathMatcher AMOUNT_MATCHER("/amount");
static const JsonParserPathMatcher PAGING_TOKEN_MATCHER("/paging_token");
static const JsonParserPathMatcher ID_MATCHER("/id");
static const JsonParserPathMatcher SOURCE_AMOUNT_MATCHER("/source_amount");

// Source asset for path payment
static const JsonParserPathMatcher SOURCE_ASSET_TYPE_MATCHER("/source_asset_type");
static const JsonParserPathMatcher SOURCE_ASSET_CODE_MATCHER("/source_asset_code");
static const JsonParserPathMatcher SOURCE_ASSET_ISSUER_MATCHER("/source_asset_issuer");

#define SOURCE_ASSET (_operation->sourceAsset.isEmpty() ? _operation->sourceAsset = stellar::Asset() : _operation->sourceAsset).getValue()

namespace ledger {
    namespace core {

        bool HorizonOperationParser::Null() {
            return true;
        }

        bool HorizonOperationParser::Bool(bool b) {
            if (_path.match(SUCCESSFUL_MATCHER)) {
                _operation->transactionSuccessful = b;
            }
            return true;
        }

        bool HorizonOperationParser::Int(int i) {
            return true;
        }

        bool HorizonOperationParser::Uint(unsigned i) {
            return true;
        }

        bool HorizonOperationParser::Int64(int64_t i) {
            return true;
        }

        bool HorizonOperationParser::Uint64(uint64_t i) {
            return true;
        }

        bool HorizonOperationParser::Double(double d) {
            return true;
        }

        bool
        HorizonOperationParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(TYPE_MATCHER)) {
                _operation->type = (stellar::OperationType) BigInt::fromString(std::string(str, length)).toInt();
            }
            return true;
        }

        bool HorizonOperationParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            if (_path.match(ID_MATCHER)) {
                _operation->id = std::string(str, length);
            } else if (_path.match(AMOUNT_MATCHER)) {
                _operation->amount = BigInt::fromFloatString(std::string(str, length), 7);
            } else if (_path.match(STARTING_BALANCE_MATCHER)) {
                _operation->amount = BigInt::fromFloatString(std::string(str, length), 7);
            } else if (_path.match(FROM_MATCHER)) {
                _operation->from = std::string(str, length);
            } else if (_path.match(TO_MATCHER)) {
                _operation->to = std::string(str, length);
            } else if (_path.match(ACCOUNT_MATCHER)) {
                _operation->to = std::string(str, length);
            } else if (_path.match(FUNDER_MATCHER)) {
                _operation->from = std::string(str, length);
            } else if (_path.match(DATE_MATCHER)) {
                _operation->createdAt = DateUtils::fromJSON(std::string(str, length));
            } else if (_path.match(PAGING_TOKEN_MATCHER)) {
                _operation->pagingToken = std::string(str, length);
            } else if (_path.match(TRANSACTION_HASH_MATCHER)) {
                _operation->transactionHash = std::string(str, length);
            } else if (_path.match(SOURCE_AMOUNT_MATCHER)) {
                _operation->sourceAmount = BigInt::fromFloatString(std::string(str, length), 7);
            } else if (_path.match(SOURCE_ASSET_CODE_MATCHER)) {
                SOURCE_ASSET.code = std::string(str, length);
            } else if (_path.match(SOURCE_ASSET_TYPE_MATCHER)) {
                SOURCE_ASSET.type = std::string(str, length);
            } else if (_path.match(SOURCE_ASSET_ISSUER_MATCHER)) {
                SOURCE_ASSET.issuer = std::string(str, length);
            } else {
                _assetParser.String(str, length, copy);
            }
            return true;
        }

        bool HorizonOperationParser::StartObject() {
            return true;
        }

        bool HorizonOperationParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            return true;
        }

        bool HorizonOperationParser::EndObject(rapidjson::SizeType memberCount) {
            return true;
        }

        bool HorizonOperationParser::StartArray() {
            return true;
        }

        bool HorizonOperationParser::EndArray(rapidjson::SizeType elementCount) {
            return true;
        }

        void HorizonOperationParser::init(stellar::Operation *operation) {
            _operation = operation;
            _assetParser.init(&_operation->asset);
        }

        void HorizonOperationParser::setPathView(const JsonParserPathView &path) {
            _path = path;
            _assetParser.setPathView(path);
        }
    }
}