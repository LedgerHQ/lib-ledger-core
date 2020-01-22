/*
 *
 * BitcoinLikeTransaction
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

#pragma once

// #include <core/operation/Operation.hpp>
#include <core/api/EstimatedSize.hpp>
#include <core/api/KeychainEngines.hpp>

#include <bitcoin/api/BitcoinLikeBlock.hpp>
#include <bitcoin/api/BitcoinLikeSignature.hpp>
#include <bitcoin/api/BitcoinLikeSignatureState.hpp>
#include <bitcoin/api/BitcoinLikeTransaction.hpp>
#include <bitcoin/block/BitcoinLikeBlock.hpp>
#include <bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <bitcoin/io/BitcoinLikeInput.hpp>
#include <bitcoin/io/BitcoinLikeOutput.hpp>
#include <bitcoin/io/BitcoinLikeWritableInput.hpp>

namespace ledger {
    namespace core {

        struct BitcoinLikePreparedInput {
            uint32_t sequence;
            std::string address;
            std::string previousTxHash;
            int32_t outputIndex;
            std::vector<std::vector<uint8_t>> pubKeys;
            BitcoinLikeBlockchainExplorerOutput output;

            BitcoinLikePreparedInput() = default;

            BitcoinLikePreparedInput(uint32_t sequence_,
                                     const std::string &address_,
                                     const std::string &previousTxHash_,
                                     int32_t outputIndex_,
                                     std::vector<std::vector<uint8_t>> pubKeys_,
                                     BitcoinLikeBlockchainExplorerOutput output_) : sequence(sequence_),
                                                                                      address(address_),
                                                                                      previousTxHash(previousTxHash_),
                                                                                      outputIndex(outputIndex_),
                                                                                      pubKeys(pubKeys_) {
                output.value = output_.value;
                output.index = output_.index;
                output.address = output_.address;
                output.script = output_.script;
                output.transactionHash = output_.transactionHash;
                output.time = output_.time;

            }
        };

        class BytesWriter;

        class BitcoinLikeTransaction : public api::BitcoinLikeTransaction {
        public:
            explicit BitcoinLikeTransaction(const api::Currency &currency,
                                            const std::string &keychainEngine = api::KeychainEngines::BIP32_P2PKH,
                                            uint64_t currentBlockHeight = 0);

            BitcoinLikeTransaction(
                const std::shared_ptr<BitcoinLikeOperation>& operation,
                const api::Currency &currency,
                const std::string &keychainEngine = api::KeychainEngines::BIP32_P2PKH,
                uint64_t currentBlockHeight = 0);

            std::vector<std::shared_ptr<api::BitcoinLikeInput>> getInputs() override;

            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> getOutputs() override;

            std::shared_ptr<api::BitcoinLikeBlock> getBlock() override;

            int64_t getLockTime() override;

            std::shared_ptr<api::Amount> getFees() override;

            std::string getHash() override;

            std::chrono::system_clock::time_point getTime() override;

            optional<int32_t> getTimestamp() override;

            std::vector<uint8_t> serialize() override;

            optional<std::vector<uint8_t>> getWitness() override;

            api::EstimatedSize getEstimatedSize() override;

            std::vector<uint8_t> serializeOutputs() override;

            int32_t getVersion() override;
            
            api::BitcoinLikeSignatureState setSignatures(const std::vector<api::BitcoinLikeSignature> & signatures, bool override = false) override;

            api::BitcoinLikeSignatureState setDERSignatures(const std::vector<std::vector<uint8_t>> & signatures, bool override = false) override;

            BitcoinLikeTransaction &addInput(const std::shared_ptr<BitcoinLikeWritableInput> &input);

            BitcoinLikeTransaction &addOutput(const std::shared_ptr<api::BitcoinLikeOutput> &output);

            BitcoinLikeTransaction &setLockTime(uint32_t lockTime);

            BitcoinLikeTransaction &setVersion(uint32_t version);

            BitcoinLikeTransaction &setTimestamp(uint32_t timestamp);

            BitcoinLikeTransaction &setHash(const std::string &hash);


            static std::shared_ptr<api::BitcoinLikeTransaction> parseRawTransaction(const api::Currency &currency,
                                                                                    const std::vector<uint8_t> &rawTransaction,
                                                                                    std::experimental::optional<int32_t> currentBlockHeight,
                                                                                    bool isSigned);

            static std::shared_ptr<api::BitcoinLikeTransaction> parseRawSignedTransaction(const api::Currency &currency,
                                                                                          const std::vector<uint8_t> &rawTransaction,
                                                                                          std::experimental::optional<int32_t> currentBlockHeight);

            static api::EstimatedSize estimateSize(std::size_t inputCount,
                                                   std::size_t outputCount,
                                                   const api::Currency &currency,
                                                   const std::string &keychainEngine);

        private:
            inline bool isWriteable() const;

            inline bool isReadOnly() const;

            inline void serializeProlog(BytesWriter &out);

            inline void serializeInputs(BytesWriter &out);

            inline void serializeOutputs(BytesWriter &out);

            inline void serializeEpilogue(BytesWriter &out);

        private:
            int32_t _version;
            std::vector<std::shared_ptr<api::BitcoinLikeInput>> _inputs;
            std::vector<std::shared_ptr<api::BitcoinLikeOutput>> _outputs;
            int32_t _lockTime;
            std::shared_ptr<api::Amount> _fees;
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<BitcoinLikeBlock> _block;
            std::string _hash;
            api::Currency _currency;
            api::BitcoinLikeNetworkParameters _params;
            Option<uint32_t> _timestamp;
            bool _writable;
            std::string _keychainEngine;
            uint64_t _currentBlockHeight;
        };
    }
}
