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
#include <bytes/BytesReader.h>
#include <bytes/RLP/RLPListEncoder.h>
#include <bytes/RLP/RLPStringEncoder.h>
#include <utils/hex.h>

namespace ledger {
    namespace core {

        EthereumLikeTransactionApi::EthereumLikeTransactionApi(const api::Currency& currency) {
            _currency = currency;
            _status = 0;
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

            _currency = operation->getAccount()->getWallet()->getCurrency();

            _gasPrice = std::make_shared<Amount>(_currency, 0, tx.gasPrice);
            _gasLimit = std::make_shared<Amount>(_currency, 0, tx.gasLimit);
            _gasUsed = std::make_shared<Amount>(_currency, 0, tx.gasUsed.getValue());
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _nonce = std::make_shared<BigInt>((int64_t)tx.nonce);
            _data = tx.inputData;
            _status = tx.status;
            _receiver = EthereumLikeAddress::fromEIP55(tx.receiver, _currency);
            _sender = EthereumLikeAddress::fromEIP55(tx.sender, _currency);

            auto vBigInt = BigInt(_currency.ethereumLikeNetworkParameters.value().ChainID) * BigInt(2) + BigInt(36);
            _vSignature = hex::toByteArray(vBigInt.toHexString());
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

        std::shared_ptr<api::Amount> EthereumLikeTransactionApi::getGasUsed() {
            return _gasUsed;
        }
        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeTransactionApi::getReceiver() {
            return _receiver;
        }

        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeTransactionApi::getSender() {
            return _sender;
        }

        std::shared_ptr<api::Amount> EthereumLikeTransactionApi::getValue() {
            return _value;
        }
        
        std::experimental::optional<std::vector<uint8_t>> EthereumLikeTransactionApi::getData() {
            return _data;
        }

        int32_t EthereumLikeTransactionApi::getStatus() {
           return _status;
        }

        std::chrono::system_clock::time_point EthereumLikeTransactionApi::getDate() {
            return _time;
        }

        std::shared_ptr<api::EthereumLikeBlock> EthereumLikeTransactionApi::getBlock() {
            return _block;
        }

        void EthereumLikeTransactionApi::setSignature(const std::vector<uint8_t> & vSignature, const std::vector<uint8_t> & rSignature, const std::vector<uint8_t> & sSignature) {
            _vSignature = vSignature;
            _rSignature = rSignature;
            _sSignature = sSignature;
        }

        void EthereumLikeTransactionApi::setDERSignature(const std::vector<uint8_t> & signature) {
            BytesReader reader(signature);
            //DER prefix
            reader.readNextByte();
            //Total length
            reader.readNextVarInt();
            //Nb of elements for R
            reader.readNextByte();
            //R length
            auto rSize = reader.readNextVarInt();
            if (rSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _rSignature = reader.read(rSize - 1);
            } else {
                _rSignature = reader.read(rSize);
            }
            //Nb of elements for S
            reader.readNextByte();
            //S length
            auto sSize = reader.readNextVarInt();
            if (sSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _sSignature = reader.read(sSize - 1);
            } else {
                _sSignature = reader.read(sSize);
            }
        }

        void EthereumLikeTransactionApi::setVSignature(const std::vector<uint8_t> & vSignature) {
            _vSignature = vSignature;
        }

        std::vector<uint8_t> EthereumLikeTransactionApi::serialize() {
            //Construct RLP object from tx
            //TODO:  need forEIP155 ?
            bool forEIP155 = true;
            RLPListEncoder txList;
            std::vector<uint8_t> empty;
            if (_nonce->toUint64() == 0) {
                txList.append(empty);
            } else {
                txList.append(hex::toByteArray(_nonce->toHexString()));
            }
            BigInt gasPrice(_gasPrice->toString());
            txList.append(hex::toByteArray(gasPrice.toHexString()));
            BigInt gasLimit(_gasLimit->toString());
            txList.append(hex::toByteArray(gasLimit.toHexString()));
            auto receiver = _receiver->toEIP55();
            auto sReceiver = receiver.substr(2,receiver.size() - 2);
            txList.append(hex::toByteArray(sReceiver));
            BigInt value(_value->toString());
            if (_value->toBigInt()->intValue() == 0) {
                txList.append(empty);
            } else {
                txList.append(hex::toByteArray(value.toHexString()));
            }
            txList.append(_data);

            if (!_rSignature.empty() && !_sSignature.empty()) {
                txList.append(_vSignature);
                txList.append(_rSignature);
                txList.append(_sSignature);
            } else {
                txList.append(hex::toByteArray(_currency.ethereumLikeNetworkParameters.value().ChainID));
                txList.append(empty);
                txList.append(empty);
            }

            BytesWriter writer;
            writer.writeByteArray(txList.encode());
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
            if (!nonce) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransactionApi::setGasLimit: Invalid Nonce");
            }
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

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setStatus(int32_t status) {
            _status = status;
            return *this;
        }

        EthereumLikeTransactionApi & EthereumLikeTransactionApi::setReceiver(const std::string &receiver) {
            _receiver = EthereumLikeAddress::fromEIP55(receiver, _currency);
            return *this;
        }

        
    }
}
