/*
 *
 * BitcoinLikeTransactionApi
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/07/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "BitcoinLikeTransactionApi.h"
#include <bytes/BytesWriter.h>
#include <bytes/BytesReader.h>
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>
#include <wallet/bitcoin/networks.hpp>
#include <crypto/HASH160.hpp>
#include <math/Base58.hpp>
#include <api/KeychainEngines.hpp>

namespace ledger {
    namespace core {


        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const api::Currency &currency,
                                                             bool isSegwit,
                                                             uint64_t currentBlockHeight) :
                _currency(currency), _isSegwit(isSegwit), _currentBlockHeight(currentBlockHeight) {
            _version = 1;
            _writable = true;
        }

        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi> &operation)
                : BitcoinLikeTransactionApi(operation->getCurrency()) {

            auto &tx = operation->getBackend().bitcoinTransaction.getValue();
            _time = tx.receivedAt;
            _lockTime = tx.lockTime;
            _writable = false;

            if (tx.fees.nonEmpty())
                _fees = std::make_shared<Amount>(operation->getAccount()->getWallet()->getCurrency(), 0,
                                                 tx.fees.getValue());
            else
                _fees = nullptr;

            auto inputCount = tx.inputs.size();
            for (auto index = 0; index < inputCount; index++) {
                _inputs.push_back(std::make_shared<BitcoinLikeInputApi>(operation, index));
            }

            auto outputCount = tx.outputs.size();
            for (auto index = 0; index < outputCount; index++) {
                _outputs.push_back(std::make_shared<BitcoinLikeOutputApi>(operation, index));
            }

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<BitcoinLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }
            _hash = tx.hash;
        }

        std::vector<std::shared_ptr<api::BitcoinLikeInput>> BitcoinLikeTransactionApi::getInputs() {
            return _inputs;
        }

        std::vector<std::shared_ptr<api::BitcoinLikeOutput>> BitcoinLikeTransactionApi::getOutputs() {
            return _outputs;
        }

        std::shared_ptr<api::BitcoinLikeBlock> BitcoinLikeTransactionApi::getBlock() {
            return _block;
        }

        int64_t BitcoinLikeTransactionApi::getLockTime() {
            return _lockTime;
        }

        std::shared_ptr<api::Amount> BitcoinLikeTransactionApi::getFees() {
            ledger::core::BigInt value(0);
            for (auto &input : getInputs()) {
                auto v = std::dynamic_pointer_cast<ledger::core::Amount>(input->getValue());
                if (v == nullptr)
                    return nullptr;
                value = value + *(v->value());
            }
            for (auto &output : getOutputs()) {
                auto v = std::dynamic_pointer_cast<ledger::core::Amount>(output->getValue())->value();
                value = value - *v;
            }
            return std::make_shared<ledger::core::Amount>(_currency, 0, value);
        }

        std::chrono::system_clock::time_point BitcoinLikeTransactionApi::getTime() {
            return _time;
        }

        std::string BitcoinLikeTransactionApi::getHash() {
            return _hash;
        }

        optional<int32_t> BitcoinLikeTransactionApi::getTimestamp() {
            return _timestamp.map<int32_t>([](const uint32_t &v) {
                return (int32_t) v;
            }).toOptional();
        }

        std::vector<uint8_t> BitcoinLikeTransactionApi::serialize() {
            BytesWriter writer;
            serializeProlog(writer);
            serializeInputs(writer);
            serializeOutputs(writer);
            serializeEpilogue(writer);
            return writer.toByteArray();
        }

        optional<std::vector<uint8_t>> BitcoinLikeTransactionApi::getWitness() {
            bool isDecred =  _currency.bitcoinLikeNetworkParameters.value().Identifier == "dcr";
            BytesWriter witness;

            //Decred has a special witness with nb of witnesses and no stack size,
            //and additional fields: amount, block height, block index
            if (_isSegwit && !isDecred) {
                for (auto &input : _inputs) {
                    auto scriptSig = input->getScriptSig();
                    if (!scriptSig.empty()) {
                        //Stack size
                        witness.writeVarInt(2);
                        witness.writeByteArray(scriptSig);
                    }
                }
            } else if (isDecred){
                witness.writeVarInt(_inputs.size());
                for (auto &input : _inputs) {
                    //TODO: Amount (not used can be anything)
                    witness.writeByteArray({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
                    witness.writeByteArray({0x00, 0x00, 0x00, 0x00});
                    witness.writeByteArray({0xff, 0xff, 0xff, 0xff});
                    auto scriptSig = input->getScriptSig();
                    witness.writeVarInt(scriptSig.size());
                    if (!scriptSig.empty()) {
                        witness.writeByteArray(scriptSig);
                    }
                    auto pubKeys = input->getPublicKeys();
                    if (!pubKeys.empty()) {
                        witness.writeVarInt(pubKeys[0].size());
                        witness.writeByteArray(pubKeys[0]);
                    }
                }
            }
            return Option<std::vector<uint8_t>>(witness.toByteArray()).toOptional();
        }

        api::EstimatedSize BitcoinLikeTransactionApi::getEstimatedSize() {
            return estimateSize(getInputs().size(), getOutputs().size(),
                                _currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction, _isSegwit);
        }

        bool BitcoinLikeTransactionApi::isWriteable() const {
            return _writable;
        }

        bool BitcoinLikeTransactionApi::isReadOnly() const {
            return !isWriteable();
        }

        BitcoinLikeTransactionApi &
        BitcoinLikeTransactionApi::addInput(const std::shared_ptr<BitcoinLikeWritableInputApi> &input) {
            _inputs.push_back(input);
            return *this;
        }

        BitcoinLikeTransactionApi &BitcoinLikeTransactionApi::setLockTime(uint32_t lockTime) {
            _lockTime = lockTime;
            return *this;
        }

        BitcoinLikeTransactionApi &
        BitcoinLikeTransactionApi::addOutput(const std::shared_ptr<api::BitcoinLikeOutput> &output) {
            _outputs.push_back(output);
            return *this;
        }

        api::EstimatedSize
        BitcoinLikeTransactionApi::estimateSize(std::size_t inputCount, std::size_t outputCount, bool hasTimestamp,
                                                bool useSegwit) {
            // TODO Handle outputs and input for multisig P2SH
            size_t maxSize, minSize, fixedSize = 0;

            // Fixed size computation
            fixedSize = 4; // Transaction version
            if (hasTimestamp)
                fixedSize += 4; // Timestamp
            fixedSize += BytesWriter().writeVarInt(inputCount).toByteArray().size(); // Number of inputs
            fixedSize += BytesWriter().writeVarInt(outputCount).toByteArray().size(); // Number of outputs
            fixedSize += 4; // Timelock

            minSize = fixedSize;
            maxSize = fixedSize;

            // Outputs size
            minSize += 32 * outputCount;
            maxSize += 34 * outputCount;

            fixedSize = fixedSize + (34 * outputCount);
            if (useSegwit) {
                fixedSize += 2; // Flag and marker size (one byte each)
                size_t noWitness, maxWitness, minWitness = 0;
                noWitness = fixedSize + (59 * inputCount);
                minWitness = noWitness + (106 * inputCount);
                maxWitness = noWitness + (108 * inputCount);
                minSize += (noWitness * 3 + minWitness) / 4;
                maxSize += (noWitness * 3 + maxWitness) / 4;
            } else {
                minSize += 146 * inputCount;
                maxSize += 148 * inputCount;
            }
            return api::EstimatedSize(static_cast<int32_t>(minSize), static_cast<int32_t>(maxSize));
        }

        BitcoinLikeTransactionApi &BitcoinLikeTransactionApi::setVersion(uint32_t version) {
            _version = version;
            return *this;
        }

        BitcoinLikeTransactionApi &BitcoinLikeTransactionApi::setTimestamp(uint32_t ts) {
            _timestamp = Option<uint32_t>(ts);
            return *this;
        }

        std::vector<uint8_t> BitcoinLikeTransactionApi::serializeOutputs() {
            BytesWriter writer;
            serializeOutputs(writer);
            return writer.toByteArray();
        }

        int32_t BitcoinLikeTransactionApi::getVersion() {
            return _version;
        }

        void BitcoinLikeTransactionApi::serializeProlog(BytesWriter &writer) {

            auto &additionalBIPs = _currency.bitcoinLikeNetworkParameters.value().AdditionalBIPs;
            auto it = std::find(additionalBIPs.begin(), additionalBIPs.end(), "ZIP");
            auto zipParameters = _currentBlockHeight > networks::ZIP_SAPLING_PARAMETERS.blockHeight ?
                                 networks::ZIP_SAPLING_PARAMETERS : networks::ZIP143_PARAMETERS;

            if (it != additionalBIPs.end() && _currentBlockHeight > networks::ZIP143_PARAMETERS.blockHeight) {
                setVersion(zipParameters.version);

                //New version and overwinter flag
                auto header = zipParameters.overwinterFlag;
                header.push_back(0x00);
                header.push_back(0x00);
                header.push_back(zipParameters.version);
                //Push header (0x80000003 in LE)
                writer.writeLeByteArray(header);

                //Version group Id (0x03C48270 in LE)
                writer.writeLeByteArray(zipParameters.versionGroupId);

            } else {
                writer.writeLeValue<int32_t>(_version);
            }

            if (_currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction) {
                auto ts = getTimestamp();
                if (!ts)
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing transaction timestamp");
                writer.writeLeValue<uint32_t>(ts.value());
            }

            if (_isSegwit) {
                //write marker
                writer.writeByte(0x00);
                //write flag
                writer.writeByte(0x01);
            }
        }

        void BitcoinLikeTransactionApi::serializeInputs(BytesWriter &writer) {
            // If all inputs are empty we need to create an unsigned transaction
            writer.writeVarInt(_inputs.size());
            for (auto &input : _inputs) {
                auto hash = input->getPreviousTxHash();
                if (!hash)
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction hash");
                writer.writeLeByteArray(hex::toByteArray(hash.value()));
                if (!input->getPreviousOutputIndex())
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction index");
                writer.writeLeValue<int32_t>(input->getPreviousOutputIndex().value());

                //Decred has only a tree field
                if (_currency.bitcoinLikeNetworkParameters.value().Identifier == "dcr") {
                    writer.writeByteArray({0x00});
                    writer.writeLeValue<uint32_t>(static_cast<const uint32_t>(input->getSequence()));
                    return;
                }

                auto scriptSig = input->getScriptSig();
                if (!_isSegwit && scriptSig.size() > 0) {
                    writer.writeVarInt(scriptSig.size());
                    writer.writeByteArray(scriptSig);
                } else if (_isSegwit && scriptSig.size() > 1) {
                    auto pubKeys = input->getPublicKeys();
                    if (!pubKeys.empty()) {
                        //TODO: handle multi-sig
                        std::vector<uint8_t> redeemScript = {0x00, 0x14};
                        redeemScript.insert(redeemScript.end(), pubKeys[0].begin(), pubKeys[0].end());
                        writer.writeVarInt(redeemScript.size() + 1);
                        writer.writeVarInt(redeemScript.size());
                        writer.writeByteArray(redeemScript);
                    } else {
                        writer.writeVarInt(0);
                    }
                } else {
                    auto prevOut = input->getPreviousOuput()->getScript();
                    writer.writeVarInt(prevOut.size());
                    writer.writeByteArray(prevOut);
                }

                writer.writeLeValue<uint32_t>(static_cast<const uint32_t>(input->getSequence()));
            }
        }

        void BitcoinLikeTransactionApi::serializeOutputs(BytesWriter &writer) {
            writer.writeVarInt(_outputs.size());

            for (auto &output : _outputs) {
                writer.writeLeValue<uint64_t>(static_cast<const uint64_t>(output->getValue()->toLong()));

                //Decred has a version of script
                if (_currency.bitcoinLikeNetworkParameters.value().Identifier == "dcr") {
                    writer.writeByteArray({0x00, 0x00});
                }

                auto script = output->getScript();
                writer.writeVarInt(script.size());
                writer.writeByteArray(script);
            }
        }

        void BitcoinLikeTransactionApi::serializeEpilogue(BytesWriter &writer) {
            auto witness = getWitness();
            auto isDecred =  _currency.bitcoinLikeNetworkParameters.value().Identifier == "dcr";

            //Decred has witness after lockTime
            if (!isDecred) {
                writer.writeByteArray(witness.value_or(std::vector<uint8_t>()));
            }

            writer.writeLeValue<int32_t>(_lockTime);

            //TODO: activate when LedgerJS not adding expiryHeight and extraData to transaction
            /*
            auto &additionalBIPs = _currency.bitcoinLikeNetworkParameters.value().AdditionalBIPs;
            auto it = std::find(additionalBIPs.begin(), additionalBIPs.end(), "ZIP");
            if (it != additionalBIPs.end() && _currentBlockHeight > networks::ZIP143_PARAMETERS.blockHeight) {
                //TODO: Feature request: set Expiry height
                writer.writeByteArray({0x00, 0x00, 0x00, 0x00});
                //Extra Data
                if (_currentBlockHeight > networks::ZIP_SAPLING_PARAMETERS.blockHeight) {
                    writer.writeByteArray({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
                } else {
                    writer.writeByteArray({0x00});
                }
            }
            */

            //Decred has a witness and an expiry height
            if (isDecred) {
                writer.writeByteArray({0xff, 0xff, 0xff, 0xff});
                writer.writeByteArray(witness.value_or(std::vector<uint8_t>()));
            }
        }


        std::shared_ptr<api::BitcoinLikeTransaction>
        api::BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                        const std::vector<uint8_t> &rawTransaction,
                                                                        std::experimental::optional<int32_t> currentBlockHeight) {
            return BitcoinLikeTransactionApi::parseRawTransaction(currency, rawTransaction, currentBlockHeight, false);
        }

        std::shared_ptr<api::BitcoinLikeTransaction>
        BitcoinLikeTransactionApi::parseRawSignedTransaction(const api::Currency &currency,
                                                             const std::vector<uint8_t> &rawTransaction,
                                                             std::experimental::optional<int32_t> currentBlockHeight) {
            return BitcoinLikeTransactionApi::parseRawTransaction(currency, rawTransaction, currentBlockHeight, true);
        }

        std::shared_ptr<api::BitcoinLikeTransaction>
        BitcoinLikeTransactionApi::parseRawTransaction(const api::Currency &currency,
                                                       const std::vector<uint8_t> &rawTransaction,
                                                       std::experimental::optional<int32_t> currentBlockHeight,
                                                       bool isSigned) {

            auto isDecred = currency.bitcoinLikeNetworkParameters.value().Identifier == "dcr";
            HashAlgorithm hashAlgorithm(currency.bitcoinLikeNetworkParameters.value().Identifier);

            BytesReader reader(rawTransaction);

            // Parse version
            auto version = reader.readNextLeUint();

            //Parse additionalBIPs if there are any
            auto &additionalBIPs = currency.bitcoinLikeNetworkParameters.value().AdditionalBIPs;
            auto it = std::find(additionalBIPs.begin(), additionalBIPs.end(), "ZIP");
            auto zipParameters = currentBlockHeight.value_or(0) > networks::ZIP_SAPLING_PARAMETERS.blockHeight ?
                                 networks::ZIP_SAPLING_PARAMETERS : networks::ZIP143_PARAMETERS;
            if (it != additionalBIPs.end() &&
                currentBlockHeight.value_or(0) > networks::ZIP143_PARAMETERS.blockHeight) {
                //Substract overwinterFlag
                auto overwinterFlag = zipParameters.overwinterFlag[0];
                version -= (~(overwinterFlag << 24) + 1);

                //Read version group Id
                reader.read(zipParameters.versionGroupId.size());
            }

            // Parse timestamp
            auto usesTimeStamp = currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction;
            uint32_t timeStamp = 0;
            if (usesTimeStamp) {
                timeStamp = reader.readNextLeUint();
            }

            // Parse segwit marker and flag
            bool isSegwit = false;
            auto marker = reader.peek();
            if (marker == 0x00) {
                isSegwit = true;
                marker = reader.readNextByte();
                auto flag = reader.readNextByte();
            }

            auto tx = std::make_shared<BitcoinLikeTransactionApi>(currency, isSegwit, currentBlockHeight.value_or(0));
            tx->setVersion(version);
            if (usesTimeStamp) {
                tx->setTimestamp(timeStamp);
            }

            // Parse inputs
            std::vector<BitcoinLikePreparedInput> preparedInputs;
            auto inputsCount = reader.readNextVarInt();
            for (auto index = 0; index < inputsCount; index++) {
                auto previousTxHash = hex::toString(reader.readNextLeBigInt(32).toByteArray());
                auto outputIndex = reader.readNextLeUint();

                ledger::core::BitcoinLikeBlockchainExplorer::Output output;
                std::string address;
                std::vector<std::vector<uint8_t>> pubKeys;

                //Decred has a tree field (1 bytes) and nothing else
                if (isDecred) {
                    reader.readNextByte();
                } else {
                    auto scriptSize = reader.readNextVarInt();
                    auto scriptSig = reader.read(scriptSize);
                    auto parsedScript = ledger::core::BitcoinLikeScript::parse(scriptSig);
                    if (parsedScript.isSuccess()) {
                        BytesReader localReader(scriptSig);
                        if (isSigned && !isSegwit) {
                            //Get address from signed script
                            auto sigSize = localReader.readNextVarInt();
                            auto sig = localReader.read(sigSize);
                            auto pubKeySize = localReader.readNextVarInt();
                            auto pubKey = localReader.read(pubKeySize);
                            pubKeys.push_back(pubKey);
                            BitcoinLikeAddress localAddress(currency, HASH160::hash(pubKey, hashAlgorithm),
                                                            currency.bitcoinLikeNetworkParameters.value().P2PKHVersion);
                            address = localAddress.toBase58();
                            output.script = hex::toString(scriptSig);
                        } else if (isSigned && isSegwit) {
                            //Get address from redeem script
                            auto redeemScriptSize = localReader.readNextVarInt();
                            auto redeemScript = localReader.read(redeemScriptSize);
                            //Get pubKeys from Redeem script : 0x00 0x14 <pubKey>
                            std::vector<uint8_t> pubKey(redeemScript.begin() + 2, redeemScript.end());
                            pubKeys.push_back(pubKey);
                            BitcoinLikeAddress localAddress(currency, HASH160::hash(redeemScript, hashAlgorithm),
                                                            currency.bitcoinLikeNetworkParameters.value().P2SHVersion);
                            address = localAddress.toBase58();

                        } else {
                            auto parsedAddress = parsedScript.getValue().parseAddress(currency);
                            if (parsedAddress.hasValue()) {
                                address = parsedAddress.getValue().toBase58();
                            }
                            output.script = hex::toString(parsedScript.getValue().serialize());
                        }
                    }
                }
                auto sequence = reader.readNextLeUint();
                output.address = address;
                output.transactionHash = previousTxHash;
                output.index = outputIndex;
                preparedInputs.emplace_back(
                        BitcoinLikePreparedInput(sequence, address, previousTxHash, outputIndex, pubKeys, output));
            }

            // Parse outputs
            auto outputsCount = reader.readNextVarInt();
            for (auto index = 0; index < outputsCount; index++) {
                ledger::core::BitcoinLikeBlockchainExplorerOutput output;
                output.index = static_cast<uint64_t>(index);
                output.value = reader.readNextLeBigInt(8);
                //Decred has an additional version script (2 byte)
                if (isDecred) {
                    reader.read(2);
                }
                auto scriptSize = reader.readNextVarInt();
                auto lockScript = reader.read(scriptSize);
                auto parsedScript = ledger::core::BitcoinLikeScript::parse(lockScript);
                if (parsedScript.isSuccess()) {
                    auto parsedAddress = parsedScript.getValue().parseAddress(currency);
                    if (parsedAddress.hasValue())
                        output.address = Option<std::string>(parsedAddress.getValue().toBase58());
                }
                output.script = hex::toString(lockScript);
                tx->addOutput(std::shared_ptr<BitcoinLikeOutputApi>(new BitcoinLikeOutputApi(
                        output, currency
                )));
            }

            //Decred has lockTime, expiry height and nb of witnesses before witness
            if (isDecred) {
                //LockTime
                tx->setLockTime(reader.readNextLeUint());
                //Expiry Height
                reader.read(4);
                //Number of inputs
                reader.readNextVarInt();
            }

            //Get witness if needed
            if (isSigned && (isSegwit || isDecred)) {
                for (auto index = 0; index < inputsCount; index++) {
                    //Decred has no stack size on witness
                    auto stackSize = isDecred ? 2 : reader.readNextVarInt();
                    if (stackSize != 0 && stackSize != 2) {
                        throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                             "Stack size not valid in signed segwit transaction");
                    }

                    if (stackSize == 2) {

                        if (isDecred) {
                            //Amount
                            reader.read(8);
                            //Block height
                            reader.read(4);
                            //Block Index
                            reader.read(4);
                            //Whole script size
                            reader.readNextVarInt();
                        }

                        auto scriptSigSize = reader.readNextVarInt();
                        auto scriptSig = reader.read(scriptSigSize);
                        auto pubKeySize = reader.readNextVarInt();
                        auto pubKey = reader.read(pubKeySize);
                        //Get address
                        /*
                         * //Hash160 of public key
                         *
                          std::vector<uint8_t> script = {0x00, 0x14};

                          auto publicKeyHash160 = HASH160::hash(pubKey);
                          script.insert(script.end(), publicKeyHash160.begin(), publicKeyHash160.end());
                          BitcoinLikeAddress address(currency, HASH160::hash(script), currency.bitcoinLikeNetworkParameters.value().P2SHVersion);
                          preparedInputs[index].output.address = address.toBase58();
                         */
                        //Get script sig
                        BytesWriter writer;
                        writer.writeVarInt(scriptSigSize);
                        writer.writeByteArray(scriptSig);
                        writer.writeVarInt(pubKeySize);
                        writer.writeByteArray(pubKey);
                        preparedInputs[index].output.script = hex::toString(writer.toByteArray());
                    }
                }
            }


            //Decred has lockTime before witness
            if (!isDecred) {
                tx->setLockTime(reader.readNextLeUint());
            }

            //Finally append inputs to tx
            for (auto i = 0; i < inputsCount; i++) {
                auto keychainEngine = isSegwit ? api::KeychainEngines::BIP49_P2SH : api::KeychainEngines::BIP32_P2PKH;
                std::vector<uint8_t> scriptSig;
                std::vector<std::vector<uint8_t>> pubKeys;
                if (isSigned) {
                    scriptSig = hex::toByteArray(preparedInputs[i].output.script);
                    pubKeys = preparedInputs[i].pubKeys;
                }
                tx->addInput(
                        std::shared_ptr<BitcoinLikeWritableInputApi>(
                                new BitcoinLikeWritableInputApi(nullptr,
                                                                nullptr,
                                                                preparedInputs[i].sequence,
                                                                pubKeys,
                                                                {},
                                                                preparedInputs[i].address,
                                                                nullptr,
                                                                preparedInputs[i].previousTxHash,
                                                                preparedInputs[i].outputIndex,
                                                                scriptSig,
                                                                std::shared_ptr<BitcoinLikeOutputApi>(
                                                                        new BitcoinLikeOutputApi(
                                                                                preparedInputs[i].output, currency)),
                                                                keychainEngine
                                )
                        )
                );
            }

            return tx;
        }

    }
}