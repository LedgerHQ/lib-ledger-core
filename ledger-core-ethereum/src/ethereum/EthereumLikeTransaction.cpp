/*
 *
 * EthereumLikeTransaction
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

#include <core/bytes/BytesWriter.hpp>
#include <core/bytes/BytesReader.hpp>
#include <core/bytes/RLP/RLPListEncoder.hpp>
#include <core/bytes/RLP/RLPStringEncoder.hpp>
#include <core/utils/Hex.hpp>
#include <core/wallet/Amount.hpp>
#include <core/wallet/AbstractAccount.hpp>
#include <core/wallet/AbstractWallet.hpp>

#include <ethereum/EthereumLikeTransaction.hpp>
#include <ethereum/EthereumLikeAddress.hpp>
#include <ethereum/EthereumNetworks.hpp>

namespace ledger {
    namespace core {

        EthereumLikeTransaction::EthereumLikeTransaction(const api::Currency& currency) {
            _currency = currency;
            _status = 0;
        }

        EthereumLikeTransaction::EthereumLikeTransaction(
            const EthereumLikeBlockchainExplorerTransaction& tx,
            const api::Currency& currency) {
            _time = tx.receivedAt;

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<EthereumLikeBlock>(tx.block.getValue());
            } else {
                _block = nullptr;
            }

            _hash = tx.hash;

            _currency = currency;

            _gasPrice = std::make_shared<Amount>(_currency, 0, tx.gasPrice);
            _gasLimit = std::make_shared<Amount>(_currency, 0, tx.gasLimit);
            _gasUsed = std::make_shared<Amount>(_currency, 0, tx.gasUsed.getValue());
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _nonce = std::make_shared<BigInt>((int64_t)tx.nonce);
            _data = tx.inputData;
            _status = tx.status;
            _receiver = EthereumLikeAddress::fromEIP55(tx.receiver, _currency);
            _sender = EthereumLikeAddress::fromEIP55(tx.sender, _currency);

            auto vBigInt = BigInt(networks::getEthereumLikeNetworkParameters(_currency.name).ChainID) * BigInt(2) + BigInt(36);
            _vSignature = hex::toByteArray(vBigInt.toHexString());
        }

        std::string EthereumLikeTransaction::getHash() {
            return _hash;
        }
        
        int32_t EthereumLikeTransaction::getNonce() {
            return (int32_t)_nonce->toUint64();
        }
        
        std::shared_ptr<api::Amount> EthereumLikeTransaction::getGasPrice() {
            return _gasPrice;
        }
        
        std::shared_ptr<api::Amount> EthereumLikeTransaction::getGasLimit() {
            return _gasLimit;
        }

        std::shared_ptr<api::Amount> EthereumLikeTransaction::getGasUsed() {
            return _gasUsed;
        }
        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeTransaction::getReceiver() {
            return _receiver;
        }

        std::shared_ptr<api::EthereumLikeAddress> EthereumLikeTransaction::getSender() {
            return _sender;
        }

        std::shared_ptr<api::Amount> EthereumLikeTransaction::getValue() {
            return _value;
        }
        
        std::experimental::optional<std::vector<uint8_t>> EthereumLikeTransaction::getData() {
            return _data;
        }

        int32_t EthereumLikeTransaction::getStatus() {
           return _status;
        }

        std::chrono::system_clock::time_point EthereumLikeTransaction::getDate() {
            return _time;
        }

        std::shared_ptr<api::EthereumLikeBlock> EthereumLikeTransaction::getBlock() {
            return _block;
        }

        void EthereumLikeTransaction::setSignature(const std::vector<uint8_t> & vSignature, const std::vector<uint8_t> & rSignature, const std::vector<uint8_t> & sSignature) {
            _vSignature = vSignature;
            _rSignature = rSignature;
            _sSignature = sSignature;
        }

        void EthereumLikeTransaction::setDERSignature(const std::vector<uint8_t> & signature) {
            BytesReader reader(signature);
            //DER prefix
            reader.readNextByte();
            //Total length
            reader.readNextVarInt();
            //Nb of elements for R
            reader.readNextByte();
            //R length
            auto rSize = reader.readNextVarInt();
            _rSignature = reader.read(rSize);
            //Nb of elements for S
            reader.readNextByte();
            //S length
            auto sSize = reader.readNextVarInt();
            _sSignature = reader.read(sSize);
        }

        void EthereumLikeTransaction::setVSignature(const std::vector<uint8_t> & vSignature) {
            _vSignature = vSignature;
        }

        std::vector<uint8_t> EthereumLikeTransaction::serialize() {
            //Construct RLP object from tx
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
                // Get rid of leading null bytes
                txList.append(_vSignature);
                txList.append(hex::toByteArray(BigInt(_rSignature, false).toHexString()));
                txList.append(hex::toByteArray(BigInt(_sSignature, false).toHexString()));
            } else {
                txList.append(hex::toByteArray(networks::getEthereumLikeNetworkParameters(_currency.name).ChainID));
                txList.append(empty);
                txList.append(empty);
            }

            BytesWriter writer;
            writer.writeByteArray(txList.encode());
            return writer.toByteArray();
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setGasPrice(const std::shared_ptr<BigInt>& gasPrice) {
            if (!gasPrice) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransaction::setGasPrice: Invalid Gas Price");
            }
            _gasPrice = std::make_shared<Amount>(_currency, 0, *gasPrice);
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setGasLimit(const std::shared_ptr<BigInt>& gasLimit) {
            if (!gasLimit) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransaction::setGasLimit: Invalid Gas Limit");
            }
            _gasLimit = std::make_shared<Amount>(_currency, 0, *gasLimit);
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setNonce(const std::shared_ptr<BigInt>& nonce) {
            if (!nonce) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransaction::setGasLimit: Invalid Nonce");
            }
            _nonce = nonce;
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setValue(const std::shared_ptr<BigInt>& value) {
            if (!value) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "EthereumLikeTransaction::setGasLimit: Invalid Value");
            }

            _value = std::make_shared<Amount>(_currency, 0, *value);
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setData(const std::vector<uint8_t> &data) {
            _data = data;
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setStatus(int32_t status) {
            _status = status;
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setReceiver(const std::string &receiver) {
            _receiver = EthereumLikeAddress::fromEIP55(receiver, _currency);
            return *this;
        }

        EthereumLikeTransaction & EthereumLikeTransaction::setSender(const std::string &sender) {
            _sender = EthereumLikeAddress::fromEIP55(sender, _currency);
            return *this;
        }

        
    }
}
