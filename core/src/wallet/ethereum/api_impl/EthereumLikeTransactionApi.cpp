/*
 *
 * EthereumLikeTransactionApi
 *
 * Created by El Khalil Bellakrid on 12/07/2018.
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


#include "EthereumLikeTransactionApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <ethereum/EthereumLikeAddress.h>
#include <bytes/BytesWriter.h>
#include <bytes/RLP/RLPListEncoder.h>
#include <bytes/RLP/RLPStringEncoder.h>

namespace ledger {
    namespace core {

        EthereumLikeTransactionApi::EthereumLikeTransactionApi(const api::Currency& currency) {
            _currency = currency;
            _version = 1;
        }

        EthereumLikeTransactionApi::EthereumLikeTransactionApi(const std::shared_ptr<OperationApi>& operation) {
            auto& tx = operation->getBackend().ethereumTransaction.getValue();
            _time = tx.receivedAt;

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<EthereumLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }

            _hash = tx.hash;

            auto currency = operation->getAccount()->getWallet()->getCurrency();

            _gasPrice = std::make_shared<Amount>(currency, 0, tx.gasPrice);
            _gasLimit = std::make_shared<Amount>(currency, 0, tx.gasLimit);
            _gasUsed = std::make_shared<Amount>(currency, 0, tx.gasUsed.getValue());
            _value = std::make_shared<Amount>(currency, 0, tx.value);

            _nonce = std::make_shared<BigInt>(tx.nonce);
            _data = tx.inputData;
            _receiver = EthereumLikeAddress::fromEIP55(tx.receiver, currency);
        }

        std::string EthereumLikeTransactionApi::getHash() {
            return _hash;
        }
        
        int32_t EthereumLikeTransactionApi::getNonce() {
            return (int32_t)_nonce->toUint64();
        }
        
        std::shared_ptr<api::Amount> EthereumLikeTransactionApi::getGasPrice() {
            return _gasPrice;
        }
        
        std::shared_ptr<api::Amount> EthereumLikeTransactionApi::getGasLimit() {
            return _gasLimit;
        }
        
        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeTransactionApi::getReceiver() {
            return _receiver;
        }
        
        std::shared_ptr<api::Amount> EthereumLikeTransactionApi::getValue() {
            return _value;
        }
        
        std::experimental::optional<std::vector<uint8_t>> EthereumLikeTransactionApi::getData() {
            return _data;
        }

        std::chrono::system_clock::time_point EthereumLikeTransactionApi::getTime() {
            return _time;
        }

        std::vector<uint8_t> EthereumLikeTransactionApi::serialize() {
            //Construct RLP object from tx
            //TODO:  need forEIP155 ?
            bool forEIP155 = true;
            auto txList = std::make_shared<RLPListEncoder>(_nonce->toString());
            txList->append(std::make_shared<RLPStringEncoder>(_gasPrice->toString()));
            txList->append(std::make_shared<RLPStringEncoder>(_gasLimit->toString()));
            auto receiver = _receiver->toEIP55();
            txList->append(std::make_shared<RLPStringEncoder>(receiver.substr(2,receiver.size() - 2)));
            txList->append(std::make_shared<RLPStringEncoder>(_value->toString()));
            txList->append(std::make_shared<RLPStringEncoder>(_data));

            //TODO:  get it from EthLikeNetworkParameters
            std::vector<uint8_t> chainID{0x01};
            txList->append(std::make_shared<RLPStringEncoder>(chainID));

            std::vector<uint8_t> zero{0x00};
            txList->append(std::make_shared<RLPStringEncoder>(zero));
            txList->append(std::make_shared<RLPStringEncoder>(zero));

            BytesWriter writer;
            writer.writeByteArray(txList->encode());
            return writer.toByteArray();
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setGasPrice(const std::shared_ptr<BigInt>& gasPrice) {
            if (!gasPrice) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransactionApi::setGasPrice: Invalid Gas Price");
            }
            _gasPrice = std::make_shared<Amount>(_currency, 0, *gasPrice);
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setGasLimit(const std::shared_ptr<BigInt>& gasLimit) {
            if (!gasLimit) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransactionApi::setGasLimit: Invalid Gas Limit");
            }
            _gasLimit = std::make_shared<Amount>(_currency, 0, *gasLimit);
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setNonce(const std::shared_ptr<BigInt>& nonce) {
            _nonce = nonce;
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setValue(const std::shared_ptr<BigInt>& value) {
            if (!value) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransactionApi::setGasLimit: Invalid Value");
            }

            _value = std::make_shared<Amount>(_currency, 0, *value);
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setData(const std::vector<uint8_t> &data) {
            _data = data;
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setReceiver(const std::string &receiver) {
            _receiver = EthereumLikeAddress::fromEIP55(receiver, _currency);
            return *this;
        }

        
    }
}
