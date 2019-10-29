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
                _protocolUpdate(protocolUpdate){
            _block = std::make_shared<TezosLikeBlockApi>(Block{});
            _type = api::TezosOperationTag::OPERATION_TAG_TRANSACTION;
        }

        TezosLikeTransactionApi::TezosLikeTransactionApi(const std::shared_ptr<OperationApi> &operation,
                                                         const std::string &protocolUpdate) {
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
        }

        api::TezosOperationTag TezosLikeTransactionApi::getType() {
            return _type;
        }

        std::string TezosLikeTransactionApi::getHash() {
            return _hash;
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getFees() {
            return _fees;
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

        void TezosLikeTransactionApi::setSignature(const std::vector<uint8_t> &signature) {
            // Signature should be 64 bytes
            if (signature.size() != 64) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setSignature: XTZ signature should have a length of 64 bytes.");
            }
            _signature = signature;
        }

        // Reference: https://www.ocamlpro.com/2018/11/21/an-introduction-to-tezos-rpcs-signing-operations/
        std::vector<uint8_t> TezosLikeTransactionApi::serialize() {
            auto isBabylonActivated = _protocolUpdate == api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON;
            BytesWriter writer;
            // Watermark: Generic-Operation
            if (_signature.empty()) {
                writer.writeByte(static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_GENERIC));
            }

            // If tx was forged then nothing to do
            if (!_rawTx.empty()) {
                writer.writeByteArray(_rawTx);
                if (!_signature.empty()) {
                    writer.writeByteArray(_signature);
                }
                return writer.toByteArray();
            }

            // Block Hash
            auto params = _currency.tezosLikeNetworkParameters.value_or(networks::getTezosLikeNetworkParameters("tezos"));
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            auto decoded = Base58::checkAndDecode(_block->getHash(), config);
            auto blockHash = decoded.getValue();
            // Remove 2 first bytes (of version)
            writer.writeByteArray(std::vector<uint8_t>{blockHash.begin() + 2, blockHash.end()});

            auto offset = static_cast<uint8_t>(isBabylonActivated ? 100 : 0);
            // Operation Tag
            writer.writeByte(static_cast<uint8_t>(_type) + offset);

            // Set Sender
            if (isBabylonActivated) {
                auto senderContractID = vector::concat({static_cast<uint8_t>(_senderCurve)}, _sender->getHash160());
                writer.writeByteArray(senderContractID);
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
            writer.writeByteArray(zarith::zSerializeNumber(_counter->toByteArray()));

            // Gas Limit
            auto bigIntGasLimit = BigInt::fromString(_gasLimit->toBigInt()->toString(10));
            writer.writeByteArray(zarith::zSerializeNumber(bigIntGasLimit.toByteArray()));

            // Storage Limit
            writer.writeByteArray(zarith::zSerializeNumber(_storage->toByteArray()));

            switch(_type) {
                case api::TezosOperationTag::OPERATION_TAG_REVEAL: {
                    if (!_signingPubKey.empty()) {
                        if (_signingPubKey.size() == 32) {
                            // Then it's ED25519
                            writer.writeByte(0x00);
                        } else { // TODO: find better, will be an issue when supporting p256
                            writer.writeByte(0x01);
                        }
                        writer.writeByteArray(_signingPubKey);
                    } else if (!_revealedPubKey.empty()) {
                        auto pKey = TezosLikeExtendedPublicKey::fromBase58(_currency, _revealedPubKey, Option<std::string>(""));
                        writer.writeByte(0x00);
                        writer.writeByteArray(pKey->derivePublicKey(""));
                    }
                    break;
                }
                case api::TezosOperationTag::OPERATION_TAG_TRANSACTION: {
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
                    if (_receiver) {
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

            // Append signature
            if (!_signature.empty()) {
                writer.writeByteArray(_signature);
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

        TezosLikeTransactionApi & TezosLikeTransactionApi::setManagerAddress(const std::string &managerAddress) {
            _managerAddress = managerAddress;
            return *this;
        }

        std::string TezosLikeTransactionApi::getManagerAddress() {
            return _managerAddress;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setRawTx(const std::vector<uint8_t> &rawTx) {
            _rawTx = rawTx;
            return *this;
        }
    }
}