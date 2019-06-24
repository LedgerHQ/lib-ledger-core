/*
 *
 * EthereumLikeTransactionParser
 *
 * Created by El Khalil Bellakrid on 14/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include "EthereumLikeTransactionParser.hpp"
#include "utils/DateUtils.hpp"
#include <math/Base58.hpp>

#define PROXY_PARSE(method, ...)                                    \
 auto& currentObject = _hierarchy.top();                            \
 if (currentObject == "block") {                                    \
    return _blockParser.method(__VA_ARGS__);                        \
 } else                                                             \

namespace ledger {
    namespace core {

        bool EthereumLikeTransactionParser::Key(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            _lastKey = std::string(str, length);

            if (_lastKey == "block" && _arrayDepth == 0) {
                auto& top = _hierarchy.top();
            }
            PROXY_PARSE(Key, str, length, copy) {
                return true;
            }
        }

        bool EthereumLikeTransactionParser::StartObject() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }

            auto& currentObject = _hierarchy.top();

            if (currentObject == "block") {
                EthereumLikeBlockchainExplorer::Block block;
                _transaction->block = Option<EthereumLikeBlockchainExplorer::Block>(block);
                _blockParser.init(&_transaction->block.getValue());
            }

            if (currentObject == "list" && _arrayDepth == 1) {
                _transaction->erc20Transactions.emplace_back(ERC20Transaction());
            }

            return true;
        }

        bool EthereumLikeTransactionParser::EndObject(rapidjson::SizeType memberCount) {
            auto& currentObject = _hierarchy.top();
            if (_lastKey == "block") {
                return false;
            }
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool EthereumLikeTransactionParser::StartArray() {
            if (_arrayDepth == 0) {
                _hierarchy.push(_lastKey);
            }
            _arrayDepth += 1;
            return true;
        }

        bool EthereumLikeTransactionParser::EndArray(rapidjson::SizeType elementCount) {
            _arrayDepth -= 1;
            if (_arrayDepth == 0) {
                _hierarchy.pop();
            }
            return true;
        }

        bool EthereumLikeTransactionParser::Null() {
            PROXY_PARSE(Null) {
                return true;
            }
        }

        bool EthereumLikeTransactionParser::Bool(bool b) {
            PROXY_PARSE(Bool, b) {
                return true;
            }
        }

        bool EthereumLikeTransactionParser::Int(int i) {
            return Uint64(i);
        }

        bool EthereumLikeTransactionParser::Uint(unsigned i) {
            return Uint64(i);
        }

        bool EthereumLikeTransactionParser::Int64(int64_t i) {
            return Uint64(i);
        }

        bool EthereumLikeTransactionParser::Uint64(uint64_t i) {
            PROXY_PARSE(Uint64, i) {
                return true;
            }
        }

        bool EthereumLikeTransactionParser::Double(double d) {
            PROXY_PARSE(Double, d) {
                return true;
            }
        }

        bool EthereumLikeTransactionParser::RawNumber(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(RawNumber, str, length, copy) {

                //TODO: this is temporary solution
                if (currentObject == "actions") {
                    return true;
                }

                std::string number(str, length);
                BigInt value = BigInt::fromString(number);
                if (_lastKey == "gas_used") {
                    _transaction->gasUsed = Option<BigInt>(value);
                }  else if (_lastKey == "gas") {
                    _transaction->gasLimit = value;
                } else if (_lastKey == "gas_price") {
                    _transaction->gasPrice = value;
                } else if (_lastKey == "confirmations") {
                    _transaction->confirmations = value.toUint64();
                } else if (_lastKey == "value") {
                    _transaction->value = value;
                } else if (_lastKey == "status") {
                    _transaction->status = value.toUint64();
                } else if (_lastKey == "count" && !_transaction->erc20Transactions.empty()) {
                    _transaction->erc20Transactions.back().value = value;
                }
                return true;
            }
        }

        bool EthereumLikeTransactionParser::String(const rapidjson::Reader::Ch *str, rapidjson::SizeType length, bool copy) {
            PROXY_PARSE(String, str, length, copy) {

                //TODO: this is temporary solution
                if (currentObject == "actions") {
                    return true;
                }

                std::string value(str, length);

                auto fromStringToBytes = [] (const std::string &data) -> std::vector<uint8_t> {
                    std::string tmpData;
                    auto pos = data.find("x");
                    if( pos != std::string::npos) { //[0x0, 0xf]
                        tmpData = data.substr(pos + 1, tmpData.size() - (pos + 1));
                    }
                    return hex::toByteArray(tmpData);
                };

                //All addresses are lower cased, we get EIP55 format
                if ((_lastKey == "from" || _lastKey == "to" || _lastKey == "contract") && value.size() > 2) {
                    value = Base58::encodeWithEIP55(value);
                }

                if (_lastKey == "hash") {
                    _transaction->hash = value;
                } else if (_lastKey == "received_at") {
                    _transaction->receivedAt = DateUtils::fromJSON(value);
                } else if (_lastKey == "to") {
                    if (currentObject == "list" && !_transaction->erc20Transactions.empty()) {
                        _transaction->erc20Transactions.back().to = value;
                    } else {
                        _transaction->receiver = value;
                    }
                } else if (_lastKey == "from") {
                    if (currentObject == "list" && !_transaction->erc20Transactions.empty()) {
                        _transaction->erc20Transactions.back().from = value;
                    } else {
                        _transaction->sender = value;
                    }
                } else if (_lastKey == "nonce") {
                    uint64_t result = 0;
                    auto nonce = fromStringToBytes(value);
                    for (auto i = 0; i < nonce.size(); i++) {
                        result += nonce[i] << i * 8;
                    }
                    _transaction->nonce = result;
                } else if (_lastKey == "input") {
                    _transaction->inputData = fromStringToBytes(value);
                } else if (_lastKey == "contract" && !_transaction->erc20Transactions.empty()) {
                    _transaction->erc20Transactions.back().contractAddress = value;
                }
                return true;
            }
        }

        EthereumLikeTransactionParser::EthereumLikeTransactionParser(std::string& lastKey) :
            _lastKey(lastKey), _blockParser(lastKey)
        {
            _arrayDepth = 0;
        }

        void EthereumLikeTransactionParser::init(EthereumLikeBlockchainExplorerTransaction *transaction) {
            _transaction = transaction;
        }

    }
}