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

namespace ledger {
    namespace core {


        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const api::Currency& currency) {
            _currency = currency;
            _version = 1;
            _writable = true;
        }

        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi> &operation) : BitcoinLikeTransactionApi(operation->getCurrency()) {

            auto& tx = operation->getBackend().bitcoinTransaction.getValue();
            _time = tx.receivedAt;
            _lockTime = tx.lockTime;
            _writable = false;

            if (tx.fees.nonEmpty())
                _fees = std::make_shared<Amount>(operation->getAccount()->getWallet()->getCurrency(), 0, tx.fees.getValue());
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
            for (auto& input : getInputs()) {
                auto v = std::dynamic_pointer_cast<ledger::core::Amount>(input->getValue());
                if (v == nullptr)
                    return nullptr;
                value = value + *(v->value());
            }
            for (auto& output : getOutputs()) {
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
            return _timestamp.map<int32_t>([] (const uint32_t& v) {
                return (int32_t)v;
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
            // TODO Implement get witness
            return Option<std::vector<uint8_t>>().toOptional();
        }

        api::EstimatedSize BitcoinLikeTransactionApi::getEstimatedSize() {
            // TODO implement segwit
            return estimateSize(getInputs().size(), getOutputs().size(), _currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction, false);
        }

        bool BitcoinLikeTransactionApi::isWriteable() const {
            return _writable;
        }

        bool BitcoinLikeTransactionApi::isReadOnly() const {
            return !isWriteable();
        }

        BitcoinLikeTransactionApi &BitcoinLikeTransactionApi::addInput(const std::shared_ptr<BitcoinLikeWritableInputApi> &input) {
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

        void BitcoinLikeTransactionApi::serializeProlog(BytesWriter &writer) {
            writer.writeLeValue<int32_t>(_version);

            if (_currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction) {
                auto ts = getTimestamp();
                if (!ts)
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing transaction timestamp");
                writer.writeLeValue<uint32_t>(ts.value());
            }
            auto witness = getWitness();
            if (witness) {
                writer.writeByte(0x00);
                writer.writeByte(0x01);
            }
        }

        void BitcoinLikeTransactionApi::serializeInputs(BytesWriter &writer) {
            // If all inputs are empty we need to create an unsigned transaction
            writer.writeVarInt(_inputs.size());
            for (auto& input : _inputs) {
                auto hash = input->getPreviousTxHash();
                if (!hash)
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction hash");
                writer.writeLeByteArray(hex::toByteArray(hash.value()));
                if (!input->getPreviousOutputIndex())
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction index");
                writer.writeLeValue<int32_t>(input->getPreviousOutputIndex().value());
                auto scriptSig = input->getScriptSig();
                if (scriptSig.size() > 0) {
                    writer.writeVarInt(scriptSig.size());
                    writer.writeByteArray(scriptSig);
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

            for (auto& output : _outputs) {
                writer.writeLeValue<uint64_t>(static_cast<const uint64_t>(output->getValue()->toLong()));
                auto script = output->getScript();
                writer.writeVarInt(script.size());
                writer.writeByteArray(script);
            }
        }

        void BitcoinLikeTransactionApi::serializeEpilogue(BytesWriter &writer) {
            writer.writeLeValue<int32_t>(_lockTime);
        }


        std::shared_ptr<api::BitcoinLikeTransaction> api::BitcoinLikeTransactionBuilder::parseRawUnsignedTransaction(
                const Currency &currency, const std::vector<uint8_t> &rawTransaction) {
            auto tx = std::make_shared<BitcoinLikeTransactionApi>(currency);
            BytesReader reader(rawTransaction);
            // Parse version
            tx->setVersion(reader.readNextLeUint());

            // Parse timestamp
            if (currency.bitcoinLikeNetworkParameters.value().UsesTimestampedTransaction) {
                tx->setTimestamp(reader.readNextLeUint());
            }

            // Parse segwit marker and flag
            auto marker = reader.peek();

            if (marker == 0x00) {
                marker = reader.readNextByte();
                auto flag = reader.readNextByte();
                // TODO Implement segwit
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Segwit transaction parsing is not handled");
            }

            // Parse inputs
            auto inputsCount = reader.readNextVarInt();
            for (auto index = 0; index < inputsCount; index++) {
                auto previousTxHash = hex::toString(reader.readNextLeBigInt(32).toByteArray());
                auto outputIndex = reader.readNextLeUint();
                auto scriptSize = reader.readNextVarInt();
                auto scriptSig = reader.read(scriptSize);
                auto sequence = reader.readNextLeUint();
                auto parsedScript = ledger::core::BitcoinLikeScript::parse(scriptSig);
                ledger::core::BitcoinLikeBlockchainExplorer::Output output;
                std::string address;
                if (parsedScript.isSuccess()) {
                   auto parsedAddress = parsedScript.getValue().parseAddress(currency);
                    if (parsedAddress.hasValue()) {
                        address = parsedAddress.getValue().toBase58();
                        output.address = address;
                    }
                }
                output.transactionHash = previousTxHash;
                output.script = hex::toString(scriptSig);
                output.index = outputIndex;
                tx->addInput(std::shared_ptr<BitcoinLikeWritableInputApi>(new BitcoinLikeWritableInputApi(
                        nullptr, nullptr, sequence, {}, {}, address, nullptr, previousTxHash, outputIndex, {},
                        std::shared_ptr<BitcoinLikeOutputApi>(new BitcoinLikeOutputApi(
                                output, currency
                        ))
                )));
            }

            auto outputsCount = reader.readNextVarInt();
            for (auto index = 0; index < outputsCount; index++) {
                ledger::core::BitcoinLikeBlockchainExplorer::Output output;
                output.index = static_cast<uint64_t>(index);
                output.value = reader.readNextLeBigInt(8);
                auto scriptSize = reader.readNextVarInt();
                auto scriptSig = reader.read(scriptSize);
                auto parsedScript = ledger::core::BitcoinLikeScript::parse(scriptSig);
                if (parsedScript.isSuccess()) {
                    auto parsedAddress = parsedScript.getValue().parseAddress(currency);
                    if (parsedAddress.hasValue())
                        output.address = Option<std::string>(parsedAddress.getValue().toBase58());
                }
                output.script = hex::toString(scriptSig);
                tx->addOutput(std::shared_ptr<BitcoinLikeOutputApi>(new BitcoinLikeOutputApi(
                    output, currency
                )));
            }

            tx->setLockTime(reader.readNextLeUint());

            // Parse outputs
            return tx;
        }

    }
}