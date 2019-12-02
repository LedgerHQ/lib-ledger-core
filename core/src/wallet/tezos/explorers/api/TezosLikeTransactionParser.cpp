/*
 *
 * TezosLikeTransactionParser
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeTransactionParser.h"
#include <wallet/currencies.hpp>
#include <api/TezosOperationTag.hpp>
#include <api/BigInt.hpp>
#define PROXY_PARSE(method, ...)                                    \
 auto& currentObject = _hierarchy.top();                            \
 if (currentObject == "block") {                                    \
    return _blockParser.method(__VA_ARGS__);                        \
 } else                                                             \

namespace ledger {
    namespace core {

        bool TezosLikeTransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(Key, str, length, copy) {
                return true;
            }
        }

        bool TezosLikeTransactionParser::StartObject() {
            _hierarchy.push(_lastKey);
            return true;
        }

        bool TezosLikeTransactionParser::EndObject(rapidjson::SizeType memberCount) {
            auto &currentObject = _hierarchy.top();
            _hierarchy.pop();
            return true;
        }

        bool TezosLikeTransactionParser::StartArray() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }
            _arrayDepth += 1;
            return true;
        }

        bool TezosLikeTransactionParser::EndArray(rapidjson::SizeType elementCount) {
            _arrayDepth -= 1;
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool TezosLikeTransactionParser::Null() {
            PROXY_PARSE(Null) {
                return true;
            }
        }

        bool TezosLikeTransactionParser::Bool(bool b) {
            PROXY_PARSE(Bool, b) {
                if ((_lastKey == "spendable" || _lastKey == "is_spendable")
                    && _transaction->originatedAccount.hasValue()) {
                    _transaction->originatedAccount.getValue().spendable = b;
                } else if ((_lastKey == "delegatable" || _lastKey == "is_delegatable")
                           && _transaction->originatedAccount.hasValue()) {
                    _transaction->originatedAccount.getValue().delegatable = b;
                } else if (_lastKey == "failed") {
                    // For Tzscan
                    _transaction->status = static_cast<uint64_t>(!b);
                } else if (_lastKey == "is_success") {
                    // For Tzstats
                    _transaction->status = static_cast<uint64_t>(b);
                }
                return true;
            }
        }

        bool TezosLikeTransactionParser::Int(int i) {
            return Uint64(i);
        }

        bool TezosLikeTransactionParser::Uint(unsigned i) {
            return Uint64(i);
        }

        bool TezosLikeTransactionParser::Int64(int64_t i) {
            return Uint64(i);
        }

        bool TezosLikeTransactionParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i) {
                return true;
            }
        }

        bool TezosLikeTransactionParser::Double(double d) {
            PROXY_PARSE(Double, d) {
                return true;
            }
        }

        bool TezosLikeTransactionParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length,
                                                   bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy) {
                std::string number(str, length);
                auto toValue = [] (const std::string &v, bool forceConversion) -> BigInt {
                    if (v.find('.') != std::string::npos || forceConversion) {
                        return BigInt(api::BigInt::fromDecimalString(v, 6, ".")->toString(10));
                    }
                    return BigInt::fromString(v);
                };
                if ((_lastKey == "op_level" || _lastKey == "height")
                    && _transaction->block.hasValue()) {
                    _transaction->block.getValue().height = BigInt::fromString(number).toUint64();
                } else if (_lastKey == "amount" || _lastKey == "volume") {
                    _transaction->value = toValue(number, _lastKey == "volume");
                } else if (_lastKey == "fee") {
                    _transaction->fees = _transaction->fees + toValue(number, false);
                } else if (_lastKey == "gas_limit") {
                    _transaction->gas_limit = toValue(number, false);
                } else if (_lastKey == "storage_limit") {
                    _transaction->storage_limit = toValue(number, false);
                } else if (_lastKey == "burned") {
                    _transaction->fees = _transaction->fees + toValue(number, true);
                }
                return true;
            }
        }

        bool
        TezosLikeTransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy) {
                std::string value(str, length);
                auto toValue = [] (const std::string &v, bool forceConversion) -> BigInt {
                    if (v.find('.') != std::string::npos || forceConversion) {
                        return BigInt(api::BigInt::fromDecimalString(v, 6, ".")->toString(10));
                    }
                    return BigInt::fromString(v);
                };

                if (_lastKey == "hash") {
                    _transaction->hash = value;
                } else if (_lastKey == "block_hash" || _lastKey == "block") {
                    TezosLikeBlockchainExplorer::Block block;
                    block.hash = value;
                    block.currencyName = currencies::TEZOS.name;
                    _transaction->block = block;
                } else if (_lastKey == "timestamp" || _lastKey == "time") {
                    auto pos = value.find('+');
                    if (pos != std::string::npos && pos > 0) {
                        value = value.substr(0, pos);
                    }
                    auto posZ = value.find('Z');
                    if (posZ == std::string::npos) {
                        value = value + "Z";
                    }
                    auto date = DateUtils::fromJSON(value);
                    _transaction->receivedAt = date;
                    if (_transaction->block.hasValue()) {
                        _transaction->block.getValue().time = date;
                    }
                } else if (_lastKey == "sender" ||
                        (currentObject == "src" && _lastKey == "tz")) {
                    _transaction->sender = value;
                } else if (_lastKey == "receiver" || _lastKey == "delegate" ||
                        ((currentObject == "destination" || currentObject == "delegate") && _lastKey == "tz")) {
                    _transaction->receiver = value;
                    if (_lastKey == "receiver" &&
                            _transaction->type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION) {
                        _transaction->originatedAccount.getValue().address = value;
                    }
                } else if (currentObject == "tz1" && _lastKey == "tz") {
                    _transaction->originatedAccount = TezosLikeBlockchainExplorerOriginatedAccount(value);
                } else if (_lastKey == "gas_limit") {
                    _transaction->gas_limit = BigInt::fromString(value);
                } else if (_lastKey == "storage_limit") {
                    _transaction->storage_limit = BigInt::fromString(value);
                } else if ((_lastKey == "kind" || _lastKey == "type") && _transaction->type == api::TezosOperationTag::OPERATION_TAG_NONE) {
                    static std::unordered_map<std::string, api::TezosOperationTag> opTags {
                            std::make_pair("reveal", api::TezosOperationTag::OPERATION_TAG_REVEAL),
                            std::make_pair("transaction", api::TezosOperationTag::OPERATION_TAG_TRANSACTION),
                            std::make_pair("origination", api::TezosOperationTag::OPERATION_TAG_ORIGINATION),
                            std::make_pair("delegation", api::TezosOperationTag::OPERATION_TAG_DELEGATION),
                    };
                    if (opTags.count(value)) {
                        _transaction->type = opTags[value];
                    } else {
                        _transaction->type = api::TezosOperationTag::OPERATION_TAG_NONE;
                    }

                    if (_lastKey == "type" &&
                            _transaction->type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION) {
                        _transaction->originatedAccount = TezosLikeBlockchainExplorerOriginatedAccount();
                    }

                } else if (_lastKey == "public_key" ||
                        (_lastKey == "data" && _transaction->type == api::TezosOperationTag::OPERATION_TAG_REVEAL)) {
                    _transaction->publicKey = value;
                } else if (_lastKey == "burn_tez") {
                    _transaction->fees = _transaction->fees + toValue(value, false);
                }
                return true;
            }
        }

        TezosLikeTransactionParser::TezosLikeTransactionParser(std::string &lastKey) :
                _lastKey(lastKey), _blockParser(lastKey) {
            _arrayDepth = 0;
        }

        void TezosLikeTransactionParser::init(TezosLikeBlockchainExplorerTransaction *transaction) {
            _transaction = transaction;
        }

    }
}