/*
 *
 * TezosLikeTransactionBuilder
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


#include "TezosLikeTransactionBuilder.h"
#include <api/TezosLikeTransactionCallback.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <bytes/zarith/zarith.h>
#include <math/Base58.hpp>
#include <api/TezosOperationTag.hpp>
#include <api/TezosCurve.hpp>

namespace ledger {
    namespace core {

        TezosLikeTransactionBuilder::TezosLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const TezosLikeTransactionBuildFunction &buildFunction) {
            _context = context;
            _currency = currency;
            _explorer = explorer;
            _build = buildFunction;
            _logger = logger;
            _request.wipe = false;
        }

        TezosLikeTransactionBuilder::TezosLikeTransactionBuilder(const TezosLikeTransactionBuilder &cpy) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setType(api::TezosOperationTag type) {
            _request.type = type;
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                   const std::string &address) {
            _request.value = std::make_shared<BigInt>(amount->toString());
            _request.toAddress = address;
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::wipeToAddress(const std::string &address) {
            _request.toAddress = address;
            _request.wipe = true;
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setFees(const std::shared_ptr<api::Amount> &fees) {
            _request.fees = std::make_shared<BigInt>(fees->toString());
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setGasLimit(const std::shared_ptr<api::Amount> & gasLimit) {
            _request.gasLimit = std::make_shared<BigInt>(gasLimit->toString());
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setStorageLimit(const std::shared_ptr<api::BigInt> & storageLimit) {
            _request.storageLimit = std::make_shared<BigInt>(storageLimit->toString(10));
            return shared_from_this();
        }

        void TezosLikeTransactionBuilder::build(const std::shared_ptr<api::TezosLikeTransactionCallback> &callback) {
            build().callback(_context, callback);
        }

        Future<std::shared_ptr<api::TezosLikeTransaction>> TezosLikeTransactionBuilder::build() {
            return _build(_request, _explorer);
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder> TezosLikeTransactionBuilder::clone() {
            return std::make_shared<TezosLikeTransactionBuilder>(*this);
        }

        void TezosLikeTransactionBuilder::reset() {
            _request = TezosLikeTransactionBuildRequest();
        }

        std::shared_ptr<api::TezosLikeTransaction>
        api::TezosLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                      const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, false);
        }

        std::shared_ptr<api::TezosLikeTransaction>
        api::TezosLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                    const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, true);
        }

        std::shared_ptr<api::TezosLikeTransaction>
        TezosLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                         const std::vector<uint8_t> &rawTransaction,
                                                         bool isSigned) {
            auto params = currency.tezosLikeNetworkParameters.value();
            auto tx = std::make_shared<TezosLikeTransactionApi>(currency);
            BytesReader reader(rawTransaction);
            // Watermark: Generic-Operation
            reader.readNextByte();

            // Block Hash
            auto blockHashBytes = reader.read(32);
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            // Magic bytes (or version ?)
            std::vector<uint8_t> blockPrefix {0x01, 0x34};
            auto blockHash = Base58::encodeWithChecksum(vector::concat(blockPrefix, blockHashBytes), config);
            tx->setBlockHash(blockHash);

            // Operation Tag
            auto OpTag = reader.readNextByte();
            tx->setType(static_cast<api::TezosOperationTag>(OpTag));

            // Get Sender
            // Originated
            auto isSenderOriginated = reader.readNextByte();
            // Curve Code
            auto senderCurveCode = reader.readNextByte();
            // sender hash160
            auto senderHash160 = reader.read(20);
            tx->setSender(std::make_shared<TezosLikeAddress>(currency,
                                                             senderHash160,
                                                             isSenderOriginated ? params.OriginatedPrefix : params.ImplicitPrefix,
                                                             Option<std::string>()));

            // Fee
            tx->setFees(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

            // Counter
            tx->setCounter(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

            // Gas Limit
            tx->setGasLimit(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

            // Storage Limit
            tx->setStorage(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

            switch (OpTag) {
                case static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_REVEAL): {
                    auto curveCode = reader.readNextByte();
                    std::vector<uint8_t> pubKey;
                    switch (curveCode) {
                        case static_cast<uint8_t>(api::TezosCurve::ED25519):
                            pubKey = reader.read(32);
                            break;
                        default:
                            pubKey = reader.read(33);
                            break;
                    }
                    tx->setSigningPubKey(pubKey);
                    tx->setValue(std::make_shared<BigInt>(BigInt::ZERO));
                    tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                       std::vector<uint8_t>(),
                                                                       std::vector<uint8_t>(),
                                                                       Option<std::string>()));
                    break;
                }
                case static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_TRANSACTION): {
                    // Amount
                    tx->setValue(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

                    // Get Receiver
                    // Originated
                    auto isReceiverOriginated = reader.readNextByte();
                    // Curve Code
                    auto receiverCurveCode = reader.readNextByte();
                    // receiver hash160
                    auto receiverHash160 = reader.read(20);
                    tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                       receiverHash160,
                                                                       isReceiverOriginated ? params.OriginatedPrefix : params.ImplicitPrefix,
                                                                       Option<std::string>()));

                    // Additional parameters
                    auto haveAdditionalParams = reader.readNextByte();
                    if (haveAdditionalParams) {
                        auto sizeAdditionals = reader.readNextBeUint();
                        auto additionals = reader.read(sizeAdditionals);
                    }
                    break;
                }
                case static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_ORIGINATION): {
                    tx->setValue(std::make_shared<BigInt>(BigInt::ZERO));
                    tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                       std::vector<uint8_t>(),
                                                                       std::vector<uint8_t>(),
                                                                       Option<std::string>()));
                    auto managerCurveCode = reader.readNextByte();
                    // manager hash160
                    auto managerHash160 = reader.read(20);
                    if (managerHash160 != senderHash160) {
                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Origination error: Manager is diffrent from sender");
                    }
                    // Balance
                    tx->setBalance(BigInt::fromHex(hex::toString(zarith::zParse(reader))));
                    // Is spendable? Is delegatable? Presence of delegate block? Presence of script block?
                    reader.read(4);
                    break;
                }
                default:
                    break;
            }
            return tx;
        }
    }
}