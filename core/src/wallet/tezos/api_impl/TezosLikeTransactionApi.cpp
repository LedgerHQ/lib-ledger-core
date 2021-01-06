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
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
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

            _transactionFees = std::make_shared<Amount>(_currency, 0, tx.fees);
            _transactionGasLimit = std::make_shared<Amount>(_currency, 0, tx.gas_limit);
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _receiver = TezosLikeAddress::fromBase58(tx.receiver, _currency);
            _sender = TezosLikeAddress::fromBase58(tx.sender, _currency);

            _type = tx.type;

            _revealedPubKey = tx.publicKey;
            _revealFees = std::make_shared<Amount>(_currency, 0, tx.fees);
            _revealGasLimit = std::make_shared<Amount>(_currency, 0, tx.gas_limit);

            _status = tx.status;
        }

        api::TezosOperationTag TezosLikeTransactionApi::getType() {
            return _type;
        }

        std::string TezosLikeTransactionApi::getHash() {
            return _hash;
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getFees() {
            int64_t fees = 0;
            if (_transactionFees) {
                fees += _transactionFees->toLong();
            }
            if (_needReveal && _revealFees) {
                fees += _revealFees->toLong();
            }
            return std::make_shared<Amount>(_currency, 0, BigInt(fees));
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getRevealFees() {
            int64_t fees = 0;
            if (_needReveal && _revealFees) {
                fees = _revealFees->toLong();
            }
            return std::make_shared<Amount>(_currency, 0, BigInt(fees));
        }

        std::shared_ptr<api::Amount> TezosLikeTransactionApi::getTransactionFees() {
            int64_t fees = 0;
            if (_transactionFees) {
                fees = _transactionFees->toLong();
            }
            return std::make_shared<Amount>(_currency, 0, BigInt(fees));
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
            int64_t gasLimit = 0;
            if (_transactionGasLimit) {
                gasLimit += _transactionGasLimit->toLong();
            }
            if (_needReveal && _revealGasLimit) {
                gasLimit += _revealGasLimit->toLong();
            }
            return std::make_shared<Amount>(_currency, 0, BigInt(gasLimit));
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
            auto decoded = signature;

            // Hackish way to seemlessly decode DER signature
            // TODO: extract this into a setDERSignature method
            if (signature.size() != 64) {
                auto der = DER::fromRaw(signature);
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
            // Remove 2 first bytes (of version)
            auto blockHash = std::vector<uint8_t>{decoded.getValue().begin() + 2, decoded.getValue().end()};
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

            if (type != api::TezosOperationTag::OPERATION_TAG_REVEAL)
            {
                // Fee
                auto bigIntFess = BigInt::fromString(_transactionFees->toBigInt()->toString(10));
                writer.writeByteArray(zarith::zSerializeNumber(bigIntFess.toByteArray()));

                // Counter
                // If account need revelation then we increment counter
                // because it was used in revelation op
                auto localCounter = _needReveal ? *_counter + BigInt(1) : *_counter;
                writer.writeByteArray(zarith::zSerializeNumber(localCounter.toByteArray()));

                // Gas Limit
                auto bigIntGasLimit = BigInt::fromString(_transactionGasLimit->toBigInt()->toString(10));
                writer.writeByteArray(zarith::zSerializeNumber(bigIntGasLimit.toByteArray()));

                // Storage Limit
                // No storage for reveal
                auto storage = _storage->toByteArray();
                writer.writeByteArray(zarith::zSerializeNumber(storage));
            }
            else {
                // Fee
                auto bigIntFess = BigInt::fromString(_revealFees->toBigInt()->toString(10));
                writer.writeByteArray(zarith::zSerializeNumber(bigIntFess.toByteArray()));

                // Counter
                auto localCounter =  *_counter;
                writer.writeByteArray(zarith::zSerializeNumber(localCounter.toByteArray()));

                // Gas Limit
                auto bigIntGasLimit = BigInt::fromString(_revealGasLimit->toBigInt()->toString(10));
                writer.writeByteArray(zarith::zSerializeNumber(bigIntGasLimit.toByteArray()));

                // Storage Limit
                // No storage for reveal
                auto storage = std::vector<uint8_t>{0};
                writer.writeByteArray(zarith::zSerializeNumber(storage));
            }

            switch(type) {
                case api::TezosOperationTag::OPERATION_TAG_REVEAL: {
                    if (!_signingPubKey.empty()) {
                        writer.writeByte(static_cast<uint8_t>(_senderCurve));
                        writer.writeByteArray(_signingPubKey);
                    } else if (!_revealedPubKey.empty()) {
                        auto pKey = TezosLikeExtendedPublicKey::fromBase58(_currency, _revealedPubKey, Option<std::string>(""));
                        writer.writeByte(static_cast<uint8_t>(_senderCurve));
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

        std::vector<uint8_t> TezosLikeTransactionApi::serializeForDryRun(const std::vector<uint8_t>& chainId) {
            BytesWriter writer;
            writer.writeByteArray(serialize());
            writer.writeByteArray(chainId);
            return writer.toByteArray();
        }

        std::string TezosLikeTransactionApi::serializeJsonForDryRun(const std::string &chainID)
        {
            using namespace rapidjson;

            Value vString(kStringType);
            Document tx;
            tx.SetObject();
            Document::AllocatorType &allocator = tx.GetAllocator();

            // Chain Id
            vString.SetString(chainID.c_str(), static_cast<SizeType>(chainID.length()), allocator);
            tx.AddMember("chain_id", vString, allocator);

            // Operation
            Value opObject(kObjectType);
            {
                // Branch
                const auto hash = _block->getHash();
                vString.SetString(hash.c_str(), static_cast<SizeType>(hash.length()), allocator);
                opObject.AddMember("branch", vString, allocator);

                // Fake sign
                static const auto bogusSignature =
                    "edsigtkpiSSschcaCt9pUVrpNPf7TTcgvgDEDD6NCEHMy8NNQJCGnMfLZzYoQj74yLjo9wx6MPVV29"
                    "CvVzgi7qEcEUok3k7AuMg";
                vString.SetString(
                    bogusSignature,
                    static_cast<SizeType>(std::strlen(bogusSignature)),
                    allocator);
                opObject.AddMember("signature", vString, allocator);

                Value opContents(kArrayType);
                {
                    if (_needReveal) {
                        Value revealOp(kObjectType);
                        {
                        static const auto transaction_type = "reveal";
                        vString.SetString(
                            transaction_type,
                            static_cast<SizeType>(std::strlen(transaction_type)),
                            allocator);
                        revealOp.AddMember("kind", vString, allocator);

                        const auto source = _sender->toBase58();
                        vString.SetString(
                            source.c_str(), static_cast<SizeType>(source.length()), allocator);
                        revealOp.AddMember("source", vString, allocator);

                        if (_revealedPubKey.empty()) {
                            if (_signingPubKey.empty()) {
                                throw make_exception(
                                    api::ErrorCode::UNSUPPORTED_OPERATION,
                                    "Json serialization of reveal operation is available only if "
                                    "revealed_pubkey or signing_pubkey is set.");
                            }
                        }
                        const auto pub_key = _revealedPubKey.empty()
                                                 ? TezosLikeExtendedPublicKey::fromRaw(
                                                       _currency,
                                                       optional<std::vector<uint8_t>>(),
                                                       _signingPubKey,
                                                       std::vector<uint8_t>(0, 32),
                                                       "",
                                                       _senderCurve)
                                                       ->toBase58()
                                                 : _revealedPubKey;
                        vString.SetString(
                            pub_key.c_str(), static_cast<SizeType>(pub_key.length()), allocator);
                        revealOp.AddMember("public_key", vString, allocator);

                        static const auto fee = "257000";
                        vString.SetString(fee, static_cast<SizeType>(std::strlen(fee)), allocator);
                        revealOp.AddMember("fee", vString, allocator);

                        const auto counter = _counter->toString();
                        vString.SetString(
                            counter.c_str(), static_cast<SizeType>(counter.length()), allocator);
                        revealOp.AddMember("counter", vString, allocator);

                        static const auto storage = "1000";
                        vString.SetString(
                            storage, static_cast<SizeType>(std::strlen(storage)), allocator);
                        revealOp.AddMember("storage_limit", vString, allocator);

                        static const auto gas = "100000";
                        vString.SetString(gas, static_cast<SizeType>(std::strlen(gas)), allocator);
                        revealOp.AddMember("gas_limit", vString, allocator);

                        }
                        opContents.PushBack(revealOp, allocator);
                    }

                    Value innerOp(kObjectType);
                    {
                    switch (_type) {
                    case api::TezosOperationTag::OPERATION_TAG_TRANSACTION: {
                        static const auto transaction_type = "transaction";
                        vString.SetString(
                            transaction_type,
                            static_cast<SizeType>(std::strlen(transaction_type)),
                            allocator);
                        innerOp.AddMember("kind", vString, allocator);

                        const auto source = _sender->toBase58();
                        vString.SetString(
                            source.c_str(), static_cast<SizeType>(source.length()), allocator);
                        innerOp.AddMember("source", vString, allocator);

                        const auto destination = _receiver->toBase58();
                        vString.SetString(
                            destination.c_str(), static_cast<SizeType>(destination.length()), allocator);
                        innerOp.AddMember("destination", vString, allocator);

                        static const auto fee = "1";
                        vString.SetString(fee, static_cast<SizeType>(std::strlen(fee)), allocator);
                        innerOp.AddMember("fee", vString, allocator);

                        // Increment the counter if the reveal was prepended
                        const auto counter = (_needReveal ? (*_counter)+BigInt::ONE : *_counter).toString();
                        vString.SetString(
                            counter.c_str(), static_cast<SizeType>(counter.length()), allocator);
                        innerOp.AddMember("counter", vString, allocator);

                        const auto amount = (_value->toLong() != 0) ?  _value->toBigInt()->toString(10) : "1";
                        vString.SetString(
                            amount.c_str(), static_cast<SizeType>(amount.length()), allocator);
                        innerOp.AddMember("amount", vString, allocator);

                        static const auto storage = "1000";
                        vString.SetString(
                            storage, static_cast<SizeType>(std::strlen(storage)), allocator);
                        innerOp.AddMember("storage_limit", vString, allocator);

                        static const auto gas = "100000";
                        vString.SetString(gas, static_cast<SizeType>(std::strlen(gas)), allocator);
                        innerOp.AddMember("gas_limit", vString, allocator);
                        break;
                    }
                    case api::TezosOperationTag::OPERATION_TAG_ORIGINATION: {
                            throw make_exception(
                                api::ErrorCode::UNSUPPORTED_OPERATION,
                                "Json serialization of origination operation is unavailable.");
                        break;
                    }
                    case api::TezosOperationTag::OPERATION_TAG_DELEGATION: {
                            throw make_exception(
                                api::ErrorCode::UNSUPPORTED_OPERATION,
                                "Json serialization of delegation operation is unavailable.");
                        break;
                    }
                    default:
                            throw make_exception(
                                api::ErrorCode::UNSUPPORTED_OPERATION,
                                "Json serialization of unknown operation type is unavailable.");
                        break;
                    }
                    }
                    opContents.PushBack(innerOp, allocator);
                }

                opObject.AddMember("contents", opContents, allocator);
            }

            tx.AddMember("operation", opObject, allocator);

// Example of valid payload in raw string
            /*
            R"json({"chain_id": "NetXdQprcVkpaWU", "operation": {
"branch": "BLq1UohguxXEdrvgxc4a4utkD1J8K4GTz2cypJqdN2nq8m1jbqW",
"contents": [{"kind": "transaction",
"source": "tz1fizckUHrisN2JXZRWEBvtq4xRQwPhoirQ",
"destination": "tz1fizckUHrisN2JXZRWEBvtq4xRQwPhoirQ", "amount":
"1432", "counter": "2531425", "fee": "1289", "gas_limit": "100000",
"storage_limit": "1000"}],
"signature":
"edsigtkpiSSschcaCt9pUVrpNPf7TTcgvgDEDD6NCEHMy8NNQJCGnMfLZzYoQj74yLjo9wx6MPVV29CvVzgi7qEcEUok3k7AuMg"}})json"
            */
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            tx.Accept(writer);
            return buffer.GetString();
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setTransactionFees(const std::shared_ptr<BigInt> &fees) {
            if (!fees) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "TezosLikeTransactionApi::setTransactionFees: Invalid Fees");
            }
            _transactionFees = std::make_shared<Amount>(_currency, 0, *fees);
            return *this;
        }

        TezosLikeTransactionApi &TezosLikeTransactionApi::setRevealFees(const std::shared_ptr<BigInt> &fees) {
            if (!fees) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "TezosLikeTransactionApi::setRevealFees: Invalid Fees");
            }
            _revealFees = std::make_shared<Amount>(_currency, 0, *fees);
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

        TezosLikeTransactionApi & TezosLikeTransactionApi::setTransactionGasLimit(const std::shared_ptr<BigInt> &gasLimit) {
            if (!gasLimit) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setTransactionGasLimit: Invalid Gas Limit");
            }
            _transactionGasLimit = std::make_shared<Amount>(_currency, 0, *gasLimit);
            return *this;
        }

        TezosLikeTransactionApi & TezosLikeTransactionApi::setRevealGasLimit(const std::shared_ptr<BigInt> &gasLimit) {
            if (!gasLimit) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "TezosLikeTransactionApi::setRevealGasLimit: Invalid Gas Limit");
            }
            _revealGasLimit = std::make_shared<Amount>(_currency, 0, *gasLimit);
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
