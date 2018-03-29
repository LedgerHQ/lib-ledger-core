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
#include "../../../../../cmake-build-debug/include/ledger/core/bytes/BytesWriter.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {


        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const api::Currency& currency) {
            _currency = currency;
            _version = 1;
        }

        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi> &operation) : BitcoinLikeTransactionApi(operation->getCurrency()) {

            auto& tx = operation->getBackend().bitcoinTransaction.getValue();
            _time = tx.receivedAt;
            _lockTime = tx.lockTime;
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
            return _fees;
        }

        std::chrono::system_clock::time_point BitcoinLikeTransactionApi::getTime() {
            return _time;
        }

        std::string BitcoinLikeTransactionApi::getHash() {
            return _hash;
        }

        optional<int32_t> BitcoinLikeTransactionApi::getTimestamp() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "optional<std::chrono::time_point> BitcoinLikeTransactionApi::getTimestamp()");
        }

        std::vector<uint8_t> BitcoinLikeTransactionApi::serialize() {
            BytesWriter writer;

            writer.writeLeValue<int32_t>(_version);
            writer.writeVarInt(_inputs.size());

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

            for (auto& input : _inputs) {
                auto hash = input->getPreviousTxHash();
                if (!hash)
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction hash");
                writer.writeByteArray(hex::toByteArray(hash.value()));
                if (!input->getPreviousOutputIndex())
                    throw make_exception(api::ErrorCode::INCOMPLETE_TRANSACTION, "Missing previous transaction index");
                writer.writeLeValue<int32_t>(input->getPreviousOutputIndex().value());
                auto scriptSig = input->getScriptSig();
                writer.writeByteArray(scriptSig);
                writer.writeLeValue<uint32_t>(static_cast<const uint32_t>(input->getSequence()));
            }

            writer.writeVarInt(_outputs.size());

            for (auto& output : _outputs) {
                writer.writeLeValue<uint64_t>(static_cast<const uint64_t>(output->getValue()->toLong()));
                auto script = output->getScript();
                writer.writeVarInt(script.size());
                writer.writeByteArray(script);
            }

            writer.writeLeValue<int32_t>(_lockTime);
            return writer.toByteArray();
        }

        optional<std::vector<uint8_t>> BitcoinLikeTransactionApi::getWitness() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "optional<std::vector<uint8_t>> BitcoinLikeTransactionApi::getWitness()");
        }

        api::EstimatedSize BitcoinLikeTransactionApi::getEstimatedSize() {
            auto witness = getWitness();
            auto inputCount = getInputs().size();
            auto outputCount = getOutputs().size();
            size_t maxSize, minSize, fixedSize, outputsSize = 0;

            // Fixed size computation
            fixedSize = 4; // Transaction version
            if (getTimestamp())
                fixedSize += 4; // Timestamp
            fixedSize += BytesWriter().writeVarInt(getInputs().size()).toByteArray().size(); // Number of inputs
            fixedSize += BytesWriter().writeVarInt(getOutputs().size()).toByteArray().size(); // Number of outputs
            fixedSize += 4; // Timelock

            // Outputs size
            for (auto& output : getOutputs()) {
                auto script = output->getScript();
                outputsSize += BytesWriter().writeVarInt(script.size()).toByteArray().size(); // Output script size
                outputsSize += script.size(); // Output script
            }

            if (witness) {
                fixedSize += 2; // Flag and marker size (one byte each)
                size_t noWitness, maxWitness, minWitness = 0;
                noWitness = fixedSize + (59 * inputCount);
                minWitness = noWitness + (106 * inputCount);
                maxWitness = noWitness + (108 * inputCount);
                minSize = (noWitness * 3 + minWitness) / 4;
                maxSize = (noWitness * 3 + maxWitness) / 4;
            } else {
                minSize = fixedSize + (146 * inputCount);
                maxSize = fixedSize + (148 * inputCount);
            }
            return api::EstimatedSize(static_cast<int32_t>(minSize), static_cast<int32_t>(maxSize));
        }
    }
}