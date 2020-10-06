/*
 *
 * TezosLikeTransactionApi
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


#include "TezosLikeTransactionApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <tezos/TezosLikeAddress.h>
#include <bytes/BytesWriter.h>
#include <bytes/BytesReader.h>
#include <crypto/DER.hpp>
#include <utils/hex.h>
#include <api_impl/BigIntImpl.hpp>
#include <wallet/tezos/tezosNetworks.h>
#include <math/Base58.hpp>
#include <bytes/zarith/zarith.h>
#include <api/TezosCurve.hpp>
#include <tezos/TezosLikeExtendedPublicKey.h>
#include <api/TezosConfigurationDefaults.hpp>
namespace ledger {
    namespace core {

        TezosLikeTransactionApi::TezosLikeTransactionApi(const api::Currency &currency,
                                                         const std::string &protocolUpdate) :
                _currency(currency),
                _protocolUpdate(protocolUpdate),
                _needReveal(false){
            _block = std::make_shared<TezosLikeBlockApi>(Block{});
            _type = api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
            _status = 0;
        }

        TezosLikeTransactionApi::TezosLikeTransactionApi(const std::shared_ptr<OperationApi> &operation,
                                                         const std::string &protocolUpdate) : _needReveal(false) {
            auto &tx = operation->getBackend().tezosTransaction.getValue();
            _time = tx.receivedAt;

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<TezosLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }

            _hash = tx.hash;

            _currency = operation->getAccount()->getWallet()->getCurrency();

            _fees = std::make_shared<Amount>(_currency, 0, tx.fees);
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _receiver = TezosLikeAddress::fromBase58(tx.receiver, _currency);
            _sender = TezosLikeAddress::fromBase58(tx.sender, _currency);

            _type = tx.type;

            _revealedPubKey = tx.publicKey;

            _status = tx.status;
        }

        api::TezosOperationTag TezosLikeTransactionApi::getType() {
            return _type;
        }

        std::string TezosLikeTransactionApi::getHash() {
            return _hash;
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getFees() {
            // Since revelation constructs a second operation in same transaction we have to double it
            return _needReveal ?
                   std::make_shared<Amount>(_currency,
                                            0 ,
                                            BigInt(_fees->toString()) * BigInt(static_cast<unsigned long long>(2))) :
                   _fees;
        }

        std::shared_ptr<api::TezosLikeAddress> TezosLikeTransactionApi::getReceiver() {
            return _receiver;
        }

        std::shared_ptr<api::TezosLikeAddress> TezosLikeTransactionApi::getSender() {
            return _sender;
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getValue() {
            return _value;
        }

        std::chrono::system_clock::time_point TezosLikeTransactionApi::getDate() {
            return _time;
        }

        std::shared_ptr<api::BigInt> TezosLikeTransactionApi::getCounter() {
            if (!_counter) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::getCounter: Invalid Counter");
            }
            return std::make_shared<api::BigIntImpl>(*_counter);
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getGasLimit() {
            return _gasLimit;
        }

        std::shared_ptr<api::BigInt> TezosLikeTransactionApi::getStorageLimit() {
            if (!_storage) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::getStorageLimit: Invalid storage limit.");
            }
            return std::make_shared<api::BigIntImpl>(*_storage);
        }

        std::experimental::optional<std::string> TezosLikeTransactionApi::getBlockHash() {
            return _block->getHash();
        }

        std::vector<uint8_t> TezosLikeTransactionApi::getSigningPubKey() {
            return _signingPubKey;
        }

        int32_t TezosLikeTransactionApi::getStatus() {
            return _status;
        }
        void TezosLikeTransactionApi::setSignature(const std::vector<uint8_t> &signature) {
            std::cout << "setSignature: " << hex::toString(signature) << std::endl;
            std::cout << "setSignature size: " << std::to_string(signature.size()) << std::endl;
            auto keyType = TezosKeyType::fromKey(signature);
            if (keyType) {
                std::cout << "Key Type: " << *keyType << std::endl;
            } else {
                std::cout << "Unknown Key Type" << std::endl;
            }

            auto decoded = signature;

            // Hackish way to seemlessly decode DER signature
            // TODO: extract this into a setDERSignature method
            if (signature.size() != 64) {
                auto der = DER::fromRaw(signature);
                std::cout << "DER parsed: " << hex::toString(der.r) << " / " << hex::toString(der.s) << std::endl;
                std::cout << "DER Sizes : R:" << std::to_string(der.r.size())
                          << " / S:" << std::to_string(der.s.size()) << std::endl;
                std::cout << "DER as bytes: " << hex::toString(der.toBytes())
                          << " (" << std::to_string(der.toBytes().size()) << ")" << std::endl;
                decoded = der.toBytes();
            }

            // Decoded bytes-only signature should be 64 bytes
            if (decoded.size() != 64) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setSignature: XTZ signature should have a length of 64 bytes.");
            }                
            _signature = decoded;
            // // This is a DER format signature
            // if (signature.size() == 70) {
            //     auto der = DER::fromRaw(signature)
            //     std::cout << "DER parsed: " << hex::toString(der.r) << " / " << hex::toString(der.s) << std::endl;
            //     std::cout << "DER Sizes : R:" << std::to_string(der.r.size()) << " / S:" << std::to_string(der.s.size()) << std::endl;
            //     std::cout << "DER as bytes: " << hex::toString(der.toBytes()) << " (" << std::to_string(der.toBytes().size()) << ")" << std::endl;
            //     _signature = der.toBytes();
            //     return;
            // }
        }

        // Reference: https://www.ocamlpro.com/2018/11/21/an-introduction-to-tezos-rpcs-signing-operations/
        std::vector<uint8_t> TezosLikeTransactionApi::serialize() {
            std::cout << "API.Serialize" << std::endl;
            BytesWriter writer;
            // Watermark: Generic-Operation
            if (_signature.empty()) {
                std::cout << "Signature is empty" << std::endl;
                writer.writeByte(static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_GENERIC));
            }

            // Block Hash
            auto params = _currency.tezosLikeNetworkParameters.value_or(networks::getTezosLikeNetworkParameters("tezos"));
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            auto decoded = Base58::checkAndDecode(_block->getHash(), config);
            // Remove 2 first bytes (of version)
            auto blockHash = std::vector<uint8_t>{decoded.getValue().begin() + 2, decoded.getValue().end()};
            std::cout << "API.serialize blockHash: " << _block->getHash()  << " - " << hex::toString(decoded.getValue()) << std::endl;
            
            std::cout << "API.serialize rawTx: " << hex::toString(_rawTx) << std::endl;
            
            // If tx was forged then nothing to do
            if (!_rawTx.empty()) {
                std::cout << "RawTX is not empty" << std::endl;
                // If we need reveal, then we must prepend it
                if (_needReveal) {
                    std::cout << "need reveal" << std::endl;
                    writer.writeByteArray(blockHash);
                    writer.writeByteArray(serializeWithType(api::TezosOperationTag::OPERATION_TAG_REVEAL));
                    // Remove branch since it's already added
                    writer.writeByteArray(std::vector<uint8_t>{_rawTx.begin() + blockHash.size(), _rawTx.end()});
                } else {
                    std::cout << "no reveal" << std::endl;
                    writer.writeByteArray(_rawTx);
                }

                if (!_signature.empty()) {
                    std::cout << "has signature" << std::endl;
                    writer.writeByteArray(_signature);
                }
                return writer.toByteArray();
            }
            std::cout << "RawTX is empty" << std::endl;

            writer.writeByteArray(blockHash);

            // If we need reveal, then we must prepend it
            if (_needReveal) {
                writer.writeByteArray(serializeWithType(api::TezosOperationTag::OPERATION_TAG_REVEAL));
            }

            writer.writeByteArray(serializeWithType(_type));

            // Append signature
            if (!_signature.empty()) {
                writer.writeByteArray(_signature);
            }

            return writer.toByteArray();
        }

        std::vector<uint8_t> TezosLikeTransactionApi::serializeWithType(api::TezosOperationTag type) {
            auto isBabylonActivated = _protocolUpdate == api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON;
            BytesWriter writer;

            auto offset = static_cast<uint8_t>(isBabylonActivated ? 100 : 0);
            // Operation Tag
            writer.writeByte(static_cast<uint8_t>(type) + offset);

            // Set Sender
            if (isBabylonActivated) {
                // After Babylon, KT need no revelation and actions from KT account means actions from manager account
                // with a smart contract, so if we are trying to reveal from KT account that means in fact that
                // manager account needs revelation
                if (type == api::TezosOperationTag::OPERATION_TAG_REVEAL && _sender->toBase58().find("KT1") == 0) {
                    auto senderContractID = vector::concat(
                            {static_cast<uint8_t>(_managerCurve)},
                            TezosLikeAddress::fromBase58(_managerAddress, _currency, Option<std::string>())->getHash160()
                    );
                    writer.writeByteArray(senderContractID);
                } else {
                    auto senderContractID = vector::concat({static_cast<uint8_t>(_senderCurve)}, _sender->getHash160());
                    writer.writeByteArray(senderContractID);
                }
            } else {
                // Originated
                auto isSenderOriginated = _sender->toBase58().find("KT1") == 0;
                writer.writeByte(static_cast<uint8_t>(isSenderOriginated));
                auto senderContractID = isSenderOriginated ?
                                        vector::concat(_sender->getHash160(), {0x00}) :
                                        vector::concat({static_cast<uint8_t>(_senderCurve)}, _sender->getHash160());
                writer.writeByteArray(senderContractID);
            }


            // Fee
            auto bigIntFess = BigInt::fromString(_fees->toBigInt()->toString(10));
            writer.writeByteArray(zarith::zSerializeNumber(bigIntFess.toByteArray()));

            // Counter
            // If account need revelation then we increment counter
            // because it was used in revelation op
            auto localCounter = _needReveal && type != api::TezosOperationTag::OPERATION_TAG_REVEAL ?
                                *_counter + BigInt(1) : *_counter;
            writer.writeByteArray(zarith::zSerializeNumber(localCounter.toByteArray()));

            // Gas Limit
            auto bigIntGasLimit = BigInt::fromString(_gasLimit->toBigInt()->toString(10));
            writer.writeByteArray(zarith::zSerializeNumber(bigIntGasLimit.toByteArray()));

            // Storage Limit
            // No storage for reveal
            auto storage = type == api::TezosOperationTag::OPERATION_TAG_REVEAL ? std::vector<uint8_t>{0} : _storage->toByteArray();
            writer.writeByteArray(zarith::zSerializeNumber(storage));

            switch(type) {
                case api::TezosOperationTag::OPERATION_TAG_REVEAL: {
                    std::cout << "case api::TezosOperationTag::OPERATION_TAG_REVEAL" << std::endl;
                    if (!_signingPubKey.empty()) {
                        std::cout << "Has signing key" << std::endl;
                        writer.writeByte(static_cast<uint8_t>(_senderCurve));
                        writer.writeByteArray(_signingPubKey);
                    } else if (!_revealedPubKey.empty()) {
                        std::cout << "Has reveal pub key" << std::endl;
                        auto pKey = TezosLikeExtendedPublicKey::fromBase58(_currency, _revealedPubKey, Option<std::string>(""));
                        writer.writeByte(static_cast<uint8_t>(_senderCurve));
                        writer.writeByteArray(pKey->derivePublicKey(""));
                    }
                    break;
                }
                case api::TezosOperationTag::OPERATION_TAG_TRANSACTION: {
                    std::cout << "case api::TezosOperationTag::OPERATION_TAG_TRANSACTION" << std::endl;
                    // Amount
                    auto bigIntValue = BigInt::fromString(_value->toBigInt()->toString(10));
                    writer.writeByteArray(zarith::zSerializeNumber(bigIntValue.toByteArray()));

                    // Set Receiver
                    // Originated
                    auto isReceiverOriginated = _receiver->toBase58().find("KT1") == 0;
                    writer.writeByte(static_cast<uint8_t>(isReceiverOriginated));
                    auto receiverContractID = isReceiverOriginated ?
                                              vector::concat(_receiver->getHash160(), {0x00}) :
                                              vector::concat({static_cast<uint8_t>(_receiverCurve)}, _receiver->getHash160());
                    writer.writeByteArray(receiverContractID);


                    // Additional parameters
                    writer.writeByte(0x00);
                    break;
                }
                case api::TezosOperationTag::OPERATION_TAG_ORIGINATION: {
                    if (!isBabylonActivated) {
                        writer.writeByte(static_cast<uint8_t >(_senderCurve));
                        writer.writeByteArray(_sender->getHash160());
                    }
                    // Balance
                    writer.writeByteArray(zarith::zSerializeNumber(_balance.toByteArray()));
                    // Is spendable ?
                    writer.writeByte(0xFF);
                    // Is delegatable ?
                    writer.writeByte(0xFF);
                    // Presence of field "delegate" ?
                    writer.writeByte(0x00);
                    // Presence of field "script" ?
                    writer.writeByte(0x00);
                    break;
                }
                case api::TezosOperationTag::OPERATION_TAG_DELEGATION: {
                    if (_receiver && !_receiver->getHash160().empty()) {
                        // Delegate is always implicit account (TBC)
                        writer.writeByte(0xFF);
                        writer.writeByte(static_cast<uint8_t >(_receiverCurve));
                        writer.writeByteArray(_receiver->getHash160());
                    } else {
                        writer.writeByte(0x00);
                    }
                    break;
                }
                default:
                    break;
            }
            return writer.toByteArray();
        }
        TezosLikeTransactionApi &TezosLikeTransactionApi::setFees(const std::shared_ptr<BigInt> &fees) {
            if (!fees) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "TezosLikeTransactionApi::setFees: Invalid Fees");
            }
            _fees = std::make_shared<Amount>(_currency, 0, *fees);
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setValue(const std::shared_ptr<BigInt> &value) {
            if (!value) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "TezosLikeTransactionApi::setValue: Invalid Value");
            }
            _value = std::make_shared<Amount>(_currency, 0, *value);
            return *this;
        }

        TezosLikeTransactionApi &
        TezosLikeTransactionApi::setSender(const std::shared_ptr<api::TezosLikeAddress> &sender, api::TezosCurve curve) {
            _senderCurve = curve;
            _sender = sender;
            return *this;
        }

        TezosLikeTransactionApi &
        TezosLikeTransactionApi::setReceiver(const std::shared_ptr<api::TezosLikeAddress> &receiver, api::TezosCurve curve) {
            _receiverCurve = curve;
            _receiver = receiver;
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setSigningPubKey(const std::vector<uint8_t> &pubKey) {
            std::cout << "Set signing pubkey: " << hex::toString(pubKey) << std::endl;
            _signingPubKey = pubKey;
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setHash(const std::string &hash) {
            _hash = hash;
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setBlockHash(const std::string &blockHash) {
            _block->setHash(blockHash);
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setGasLimit(const std::shared_ptr<BigInt> &gasLimit) {
            if (!gasLimit) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setGasLimit: Invalid Gas Limit");
            }
            _gasLimit = std::make_shared<Amount>(_currency, 0, *gasLimit);
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setCounter(const std::shared_ptr<BigInt> &counter) {
            if (!counter) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setCounter: Invalid Counter");
            }
            _counter = counter;
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setStorage(const std::shared_ptr<BigInt>& storage) {
            if (!storage) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setStorage: Invalid Storage");
            }
            _storage = storage;
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setType(api::TezosOperationTag type) {
            _type = type;
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setBalance(const BigInt &balance) {
            _balance = balance;
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setManagerAddress(const std::string &managerAddress, api::TezosCurve curve) {
            _managerAddress = managerAddress;
            _managerCurve = curve;
            return *this;
        }

        std::string TezosLikeTransactionApi::getManagerAddress() const {
            return _managerAddress;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setRawTx(const std::vector<uint8_t> &rawTx) {
            _rawTx = rawTx;
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::reveal(bool needReveal) {
            _needReveal = needReveal;
            return *this;
        }

        bool TezosLikeTransactionApi::toReveal() const {
            return _needReveal;
        }
    }
}
