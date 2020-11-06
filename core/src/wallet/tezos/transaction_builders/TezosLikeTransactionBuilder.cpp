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
#include <api/TezosConfigurationDefaults.hpp>
#include <api/TezosOperationTag.hpp>
#include <api/TezosCurve.hpp>

namespace ledger {
    namespace core {

        TezosLikeTransactionBuilder::TezosLikeTransactionBuilder(
                const std::string &senderAddress,
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const TezosLikeTransactionBuildFunction &buildFunction,
                const std::string &protocolUpdate) :
                _senderAddress(senderAddress), _context(context),
                _currency(currency), _explorer(explorer),
                _build(buildFunction), _logger(logger){
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
            // XTZ self-transactions seems to always fail and existing wallet forbid self-transactions
            // So that's why we prevent user from creating a self-transaction
            // TODO: Get reference to confirm this
            if (address == _senderAddress) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Can not send funds to sending address !");
            }
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
            _request.transactionFees = std::make_shared<BigInt>(fees->toString());
            _request.revealFees = std::make_shared<BigInt>(fees->toString());
            return shared_from_this();
        }

        std::shared_ptr<api::TezosLikeTransactionBuilder>
        TezosLikeTransactionBuilder::setGasLimit(const std::shared_ptr<api::Amount> & gasLimit) {
            _request.transactionGasLimit = std::make_shared<BigInt>(gasLimit->toString());
            _request.revealGasLimit = std::make_shared<BigInt>(gasLimit->toString());
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
                                                                      const std::vector<uint8_t> &rawTransaction,
                                                                      const std::string& protocolUpdate) {
            
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency,
                                                                                    rawTransaction,
                                                                                    false,
                                                                                    protocolUpdate
            );
        }

        std::shared_ptr<api::TezosLikeTransaction>
        api::TezosLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                    const std::vector<uint8_t> &rawTransaction,
                                                                    const std::string& protocolUpdate) {
            
            return ::ledger::core::TezosLikeTransactionBuilder::parseRawTransaction(currency,
                                                                                    rawTransaction,
                                                                                    true,
                                                                                    protocolUpdate
            );
        }

        // Reference: https://www.ocamlpro.com/2018/11/21/an-introduction-to-tezos-rpcs-signing-operations/
        std::shared_ptr<api::TezosLikeTransaction>
        TezosLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                         const std::vector<uint8_t> &rawTransaction,
                                                         bool isSigned,
                                                         const std::string &protocolUpdate) {
            auto isBabylonActivated = protocolUpdate == api::TezosConfigurationDefaults::TEZOS_PROTOCOL_UPDATE_BABYLON;
            
            auto params = currency.tezosLikeNetworkParameters.value();
            auto tx = std::make_shared<TezosLikeTransactionApi>(currency, protocolUpdate);
          
            if (isSigned) {
                //if signed remove the signature
                if (rawTransaction.size() > 64) {
                    auto raw = std::vector<uint8_t>{rawTransaction.begin(), rawTransaction.end()-64};
                    tx->setRawTx(raw);
                }
            }
            else { 
                //if not signed remove the watermark:
                if (rawTransaction.size() > 1) {
                    auto raw = std::vector<uint8_t>{rawTransaction.begin() + 1, rawTransaction.end()};
                    tx->setRawTx(raw);  
                }
            }
         
            BytesReader reader(rawTransaction);
            if (!isSigned) {
                // Watermark: Generic-Operation
                reader.readNextByte();
            }

            // Block Hash
            auto blockHashBytes = reader.read(32);

            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            // Magic bytes (or version ?)
            std::vector<uint8_t> blockPrefix {0x01, 0x34};
            auto blockHash = Base58::encodeWithChecksum(vector::concat(blockPrefix, blockHashBytes), config);
            tx->setBlockHash(blockHash);

            uint8_t OpTag;
            auto offset = static_cast<uint8_t>(isBabylonActivated ? 100 : 0);
            do {
                // Operation Tag
                OpTag = reader.readNextByte();
                tx->setType(static_cast<api::TezosOperationTag>(OpTag - offset));
                // sender hash160
                std::vector<uint8_t> senderHash160, version;
                uint8_t senderCurveCode;
                // Get Sender
                // Originated
                if (isBabylonActivated) {
                    // First curve code ...
                    senderCurveCode = reader.readNextByte();
                    // then hash160 ...
                    senderHash160 = reader.read(20);
                    version = TezosLikeAddress::getPrefixFromImplicitVersion(params.ImplicitPrefix, api::TezosCurve(senderCurveCode));
                } else {
                    auto isSenderOriginated = reader.readNextByte();
                    if (isSenderOriginated) {
                        // Then we read 20 bytes of publicKey hash
                        senderHash160 = reader.read(20);
                        // ... and padding
                        reader.readNextByte();
                        version = params.OriginatedPrefix;
                    } else {
                        // Otherwise first curve code ...
                        senderCurveCode = reader.readNextByte();
                        // then hash160 ...
                        senderHash160 = reader.read(20);
                        version = TezosLikeAddress::getPrefixFromImplicitVersion(params.ImplicitPrefix, api::TezosCurve(senderCurveCode));
                    }
                }
                tx->setSender(std::make_shared<TezosLikeAddress>(currency, senderHash160, version, Option<std::string>()),
                                static_cast<api::TezosCurve>(senderCurveCode));

                // Fee
                if(OpTag - offset == static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_REVEAL)) {
                    tx->setRevealFees(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));
                }
                else {
                    tx->setTransactionFees(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));
                }
                // Counter
                tx->setCounter(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

                // Gas Limit
                if(OpTag - offset == static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_REVEAL)) {
                    tx->setRevealGasLimit(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));
                }
                else {
                    tx->setTransactionGasLimit(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));    
                }
                // Storage Limit
                tx->setStorage(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

                // Offset introduced by Babylon
                
                switch (OpTag - offset) {
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
                                                                        Option<std::string>()),
                                        static_cast<api::TezosCurve>(curveCode));
                        tx->reveal(true); 
                        break;
                    }
                    case static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_TRANSACTION): {
                        // Amount
                        tx->setValue(std::make_shared<BigInt>(BigInt::fromHex(hex::toString(zarith::zParse(reader)))));

                        // Get Receiver
                        // Originated
                        std::vector<uint8_t> receiverHash160, receiverVersion;

                        auto isReceiverOriginated = reader.readNextByte();

                        if (!isReceiverOriginated) {
                            // Curve code
                            auto receiverCurveCode = reader.readNextByte();
                            // 20 bytes of publicKey hash
                            receiverHash160 = reader.read(20);
                            receiverVersion = TezosLikeAddress::getPrefixFromImplicitVersion(params.ImplicitPrefix, api::TezosCurve(receiverCurveCode));
                            tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                        receiverHash160,
                                                                        receiverVersion,
                                                                        Option<std::string>()),
                                            static_cast<api::TezosCurve>(receiverCurveCode));
                        } else {
                            // 20 bytes of publicKey hash
                            receiverHash160 = reader.read(20);
                            // Padding
                            reader.readNextByte();
                            receiverVersion = params.OriginatedPrefix;
                            tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                        receiverHash160,
                                                                        receiverVersion,
                                                                        Option<std::string>()));
                        }

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
                        if (!isBabylonActivated) {
                            auto managerCurveCode = reader.readNextByte();
                            // manager hash160
                            auto managerHash160 = reader.read(20);
                            if (managerHash160 != senderHash160) {
                                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Origination error: Manager is different from sender");
                            }
                        }
                        // Balance
                        tx->setBalance(BigInt::fromHex(hex::toString(zarith::zParse(reader))));
                        // Is spendable? Is delegatable? Presence of delegate block? Presence of script block?
                        reader.read(4);
                        break;
                    }
                    case static_cast<uint8_t>(api::TezosOperationTag::OPERATION_TAG_DELEGATION): {
                        tx->setValue(std::make_shared<BigInt>(BigInt::ZERO));
                        auto hasDelegationData = reader.readNextByte();
                        if (hasDelegationData) {
                            // Curve Code
                            auto delegateCurveCode = reader.readNextByte();
                            // Delegate hash160
                            auto delegateHash160 = reader.read(20);
                            tx->setReceiver(std::make_shared<TezosLikeAddress>(currency,
                                                                            delegateHash160,
                                                                            TezosLikeAddress::getPrefixFromImplicitVersion(
                                                                                    params.ImplicitPrefix,
                                                                                    api::TezosCurve(delegateCurveCode)
                                                                            ), // can't delegate to originated account (TBC)
                                                                            Option<std::string>()));
                        }
                        break;
                    }
                    default:
                        break;
                }
            } while(isSigned ? reader.available()-64 > 0 : reader.available() > 0 );
            

            // Parse signature
            if (isSigned) {
                tx->setSignature(reader.read(64));
            }

            return tx;
        }
    }
}
